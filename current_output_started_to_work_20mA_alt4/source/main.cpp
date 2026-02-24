//#define	PERFORM_MANUAL_CALIBRATION

//constexpr double	dmm_top		=  20.262;
//constexpr double	dmm_btm		= -20.342;
constexpr double	dmm_top		=  20.264;
constexpr double	dmm_btm		= -20.344;

#include	"r01lib.h"
#include	"shasta_register.h"

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
SHASTA_basic	shasta( spi, 0 );

using enum SHASTA_basic::Register16;
using enum SHASTA_basic::Register24;
using enum SHASTA_basic::Command;

constexpr double	ref_point	=  20.00;
constexpr double	dmm_gain	= 40.00 / (dmm_top - dmm_btm);
constexpr double	dmm_ofst	= ref_point - (dmm_gain * dmm_top);

constexpr int32_t	dac_code_neg20mA	= 0x614780;

#ifdef	PERFORM_MANUAL_CALIBRATION
constexpr int32_t	coef_gain	= 0x400000;
constexpr int32_t	coef_ofst	= 0;
#else
constexpr int32_t	coef_gain	= (int32_t)(dmm_gain * (double)0x400000);
constexpr int32_t	coef_ofst	= (int32_t)((dmm_ofst / 20.00) * (double)dac_code_neg20mA);
#endif

int32_t	mA2DacCode( double value )
{
	return	~(int32_t)((double)0x614780 * value / ref_point);
}

int main( void )
{
	printf( "***** Hello, SHASTA board! *****\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	shasta.command( CMD_RESET );

	printf( "ofst = %ld\r\n", coef_ofst );
	printf( "gain = %lf\r\n", dmm_gain );
	printf( "dmm_ofst = %lf\r\n", dmm_ofst );

	while ( !(shasta.reg( SYS_STATUS ) & 0x2000) )
		;
	
	printf( "0x%04X\r\n", shasta.reg( PN2 ) );
	printf( "0x%04X\r\n", shasta.reg( PN1 ) );

	printf( "gain_coeff = %lf\r\n", (double)shasta.reg( GAINCOEF1 ) / (double)0x400000 );
	printf( "ofst_coeff = %ld\r\n", shasta.reg( OFFSET_COEF1 ) );
	printf( "ofst_coeff = 0x%lX\r\n", shasta.reg( OFFSET_COEF1 ) );

	shasta.reg( GAINCOEF1,    coef_gain );
	shasta.reg( OFFSET_COEF1, coef_ofst );

	shasta.reg( AIO_CONFIG,  0x6061 );
	shasta.reg( AO_CAL_COEF, 0x1 << 12 );
	shasta.reg( AIO_PROT_CFG, 0x7 << 13 | 0x0 << 11 | 0x3 << 9 | 0x1 << 8 | 0x3 << 6 );
	shasta.reg( AO_SLR_CTRL, 0xAA00 );
	shasta.reg( AWG_PER,     0x0000 );
	shasta.reg( AO_SYSCFG,   0x0C00 );
	
//	shasta.reg( AO_DATA, dac_code_neg20mA );
	
#ifdef	PERFORM_MANUAL_CALIBRATION
	std::vector<double>	current	= { -20.00, 20.00 };
#else
	std::vector<double>	current	= { -20.00, -10.00, -2.00, 0.00, 2.00, 10.00, 20.00 };
#endif

	while ( true )
	{
		for ( auto&& c: current )
		{
			printf( "hit [return] key in terminal pane to output = %+4.1lfmA", c );
			getchar();

			shasta.reg( AO_DATA, mA2DacCode( c ) );

			wait( 1 );
			printf( "now, output is %+4.1lfmA, status = 0x%04X\r\n\r\n", c, shasta.reg( AIO_STATUS ) );
		}
	}
}
