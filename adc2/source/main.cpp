//#define	PERFORM_MANUAL_CALIBRATION

//constexpr double	dmm_top		=  20.262;
//constexpr double	dmm_btm		= -20.342;
constexpr double	dmm_top		=  20.17575;
constexpr double	dmm_btm		= -20.23865;

#include	"r01lib.h"
#include	"shasta_register.h"

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
SHASTA_basic	shasta( spi, 0 );

InterruptIn		up( SW2 );
InterruptIn		down( SW3 );

using enum SHASTA_basic::Register16;
using enum SHASTA_basic::Register24;
using enum SHASTA_basic::Command;

constexpr double	ref_point	=  20.00;
constexpr double	dmm_gain	= (ref_point * 2) / (dmm_top - dmm_btm);
constexpr double	dmm_ofst	= ref_point - (dmm_gain * dmm_top);

constexpr int32_t	dac_code_neg20mA	= 0x614780;

#ifdef	PERFORM_MANUAL_CALIBRATION
constexpr int32_t	coef_gain	= 0x400000;
constexpr int32_t	coef_ofst	= 0;
#else
constexpr int32_t	coef_gain	= (int32_t)(dmm_gain * (double)0x400000);
constexpr int32_t	coef_ofst	= (int32_t)((dmm_ofst / ref_point) * (double)dac_code_neg20mA);
#endif

volatile uint8_t	up_flag		= false;
volatile uint8_t	down_flag	= false;

void	up_callback( void )
{
	up_flag	= true;
}

void	down_callback( void )
{
	down_flag	= true;
}

int32_t	mA2DacCode( double value )
{
	return	~(int32_t)((double)0x614780 * value / ref_point);
}

void set_current( double c )
{
	shasta.reg( AO_DATA, mA2DacCode( c ) );

	wait( 0.2 );
	printf( "now, output is %+4.1lfmA, status = 0x%04X\r\n", c, shasta.reg( AIO_STATUS ) );
}

int main( void )
{
	printf( "***** Hello, SHASTA board! *****\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	shasta.command( CMD_RESET );

	while ( !(shasta.reg( SYS_STATUS ) & 0x2000) )
		;

	#if 1
	printf( "Part number          = 0x%04X%04X%02\r\n", shasta.reg( PN2 ), shasta.reg( PN1 ), shasta.reg( PN0_REV ) >> 8 );
	printf( "Revision             = 0x%02X\r\n", shasta.reg( PN0_REV ) & 0xFF );
	printf( "Unique serial number = 0x%06X%06X\r\n", shasta.reg( SERIAL1 ), shasta.reg( SERIAL0 ) );

	printf( "GAINCOEF5 = 0x%lX\r\n", shasta.reg( GAINCOEF1 )  );
	printf( "GAINCOEF5 = %lf\r\n", shasta.reg( GAINCOEF1 ) / (double)0x400000  );

	shasta.reg( GAINCOEF5,    coef_gain );
	shasta.reg( OFFSET_COEF5, coef_ofst );

#endif

	shasta.reg( AI_SYSCFG,  0x0800 );
	shasta.reg( AIO_CONFIG,  0x6002 );
	shasta.reg( AO_CAL_COEF, 0x0 << 12 );
	shasta.reg( AIO_PROT_CFG, 0x7 << 13 | 0x0 << 11 | 0x3 << 9 | 0x1 << 8 | 0x3 << 6 );
	shasta.reg( AO_SLR_CTRL, 0x8E00 );
	shasta.reg( AWG_PER,     0x0000 );
	shasta.reg( AO_SYSCFG,   0x0C00 );
	
	shasta.reg( SYS_CONFIG,  0x0000 );
	shasta.reg( CK_SRC_SEL_CONFIG,  0x0000 );

	shasta.command( CMD_CH0 );
	shasta.reg( AI_CONFIG0,  0x0024 );
	shasta.reg( AI_CONFIG1,  0x5038 );
	shasta.reg( AI_CONFIG2,  0x5000 );

	while ( true )
	{
		shasta.command( CMD_SS );
		wait( 0.1 );

			int32_t	data	= shasta.reg( AI_DATA0 );
			printf( "0x%06X, ", data & 0xFFFFFF );
			double	coeff	= ((20.00 * 2.50 / (double)(1 << 24) ));
			printf( "%lfV\r\n", (double)data * coeff );

		wait( 1.0 );
	}
}
