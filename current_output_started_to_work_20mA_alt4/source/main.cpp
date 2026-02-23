#include	"r01lib.h"
#include	"shasta_register.h"

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
SHASTA_basic	shasta( spi, 0 );


using enum SHASTA_basic::Register16;
using enum SHASTA_basic::Register24;
using enum SHASTA_basic::Command;

constexpr double	dmm_top		=  20.261;
constexpr double	dmm_btm		= -20.341;
constexpr double	dmm_gain	= 40.00 / (dmm_top - dmm_btm);
constexpr double	dmm_ofst	= dmm_top - (dmm_gain * dmm_top);

constexpr int32_t	coef_gain	= (int32_t)(dmm_gain * (double)0x400000);
constexpr int32_t	coef_ofst	= (int32_t)((dmm_ofst / 25.00) * (double)0x7FFFFF);

int main( void )
{
	printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );

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

	wait( 0.002 );
	
	printf( "gain_coeff = %lf\r\n", (double)shasta.reg( GAINCOEF1 ) / (double)0x400000 );
	printf( "ofst_coeff = %ld\r\n", shasta.reg( OFFSET_COEF1 ) );
	printf( "ofst_coeff = 0x%lX\r\n", shasta.reg( OFFSET_COEF1 ) );

	shasta.reg( GAINCOEF1, coef_gain );
	shasta.reg( OFFSET_COEF1, coef_ofst );

	shasta.reg( AIO_CONFIG, 0x6061 );	
	shasta.reg( AO_CAL_COEF, 0x1 << 12 );
	shasta.reg( AO_SLR_CTRL, 0xAA00 );
	shasta.reg( AWG_PER, 0x0000 );
	shasta.reg( AO_SYSCFG, 0x0C00 );
	
#if 1
	shasta.reg( SYS_CONFIG, 0x0000 );
	shasta.reg( CK_SRC_SEL_CONFIG, 0x0000 );
#endif
	
	shasta.reg( AO_DATA, 0x614780 );
//	shasta.reg( AO_DATA, ~0x614780 );

	
	int32_t	count	= 0;
	
	while ( true )
	{
		shasta.reg( AO_DATA, 0x614780 );
		printf( "0x%04X\r\n", shasta.reg( AIO_STATUS ) );
		wait( 2 );

		shasta.reg( AO_DATA, ~0x614780 );
		printf( "0x%04X\r\n", shasta.reg( AIO_STATUS ) );
		wait( 2 );
	}
}
