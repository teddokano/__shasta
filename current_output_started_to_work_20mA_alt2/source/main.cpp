/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/SPI_for_AFE.h"

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
SPI_for_AFE		dev( spi, 0 );

constexpr uint16_t	CMD_RESET	= 0x14;
constexpr uint16_t	SYS_STATUS	= 0x2B;

constexpr uint16_t	PN2	= 0x40;
constexpr uint16_t	PN1	= 0x41;



constexpr uint16_t	AIO_CONFIG		= 0x1C00 | 0x20;
constexpr uint16_t	AO_CAL_COEF		= 0x1C00 | 0x21;
constexpr uint16_t	AIO_PROT_CFG	= 0x1C00 | 0x22;
constexpr uint16_t	AO_SLR_CTRL		= 0x1C00 | 0x23;
constexpr uint16_t	AWG_PER			= 0x1C00 | 0x24;
constexpr uint16_t	AO_SYSCFG		= 0x1C00 | 0x25;
constexpr uint16_t	AIO_STATUS		= 0x1C00 | 0x26;

constexpr uint16_t	AO_DATA			= 0x1C00 | 0x28;
constexpr uint16_t	AO_OC_POS_LIMIT	= 0x1C00 | 0x29;
constexpr uint16_t	AO_OC_NEG_LIMIT	= 0x1C00 | 0x2A;
constexpr uint16_t	AWG_AMP_MAX		= 0x1C00 | 0x2B;
constexpr uint16_t	AWG_AMP_MIN		= 0x1C00 | 0x2C;


constexpr uint16_t	CMD_AO_ABORT	= 0x1C00 | 0x3;


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

	dev.write_r16( CMD_RESET );

	
	printf( "ofst = %ld\r\n", coef_ofst );
	printf( "gain = %lf\r\n", dmm_gain );
	printf( "dmm_ofst = %lf\r\n", dmm_ofst );


	while ( !(dev.read_r16( SYS_STATUS ) & 0x2000) )
		;
	
	printf( "0x%04X\r\n", dev.read_r16( PN2 ) );
	printf( "0x%04X\r\n", dev.read_r16( PN1 ) );

//	dev.write_r16( 0x1024, 0x0800 );
	wait( 0.002 );
	
#if 1
	dev.write_r24( 0x51, coef_gain );
	dev.write_r24( 0x59, coef_ofst );
#else
	dev.write_r24( 0x51, 0x400000 );
	dev.write_r24( 0x59, 0x000000 );	
#endif

//	dev.write_r16( AIO_CONFIG, 0x6061 );
//	dev.write_r16( AIO_CONFIG, 0x6041 );
	dev.write_r16( AIO_CONFIG, 0x6062 );
	
	dev.write_r16( AO_CAL_COEF, 0x1 << 12 );
	
	dev.write_r16( AIO_PROT_CFG, 0x7 << 13 | 0x0 << 11 | 0x3 << 9 | 0x1 << 8 | 0x3 << 6 );
	dev.write_r16( AO_SLR_CTRL, 0xAA00 );

	dev.write_r16( AWG_PER, 0x0000 );
	dev.write_r16( AO_SYSCFG, 0x0C00 );
	
#if 1
	dev.write_r16( 0x2A, 0x0000 );
	dev.write_r16( 0x30, 0x0000 );

	dev.write_r16( 0x30, 0x0000 );
#endif
	dev.write_r24( AO_OC_POS_LIMIT, (0x614780 * 5) / 4 );
//	dev.write_r24( AO_OC_NEG_LIMIT, 0x614780 );
//	dev.write_r24( AO_OC_NEG_LIMIT, 0x7FFFFF );
	
	dev.read_r16( PN2 );
	dev.write_r24( AO_DATA, 0x614780 );
//	dev.write_r24( AO_DATA, ~0x614780 );

	
	int32_t	count	= 0;
	
	while ( true )
	{
		dev.write_r24( AO_DATA, 0x614780 );
		printf( "0x%04X\r\n", dev.read_r16( AIO_STATUS ) );
		wait( 2 );

		dev.write_r24( AO_DATA, ~0x614780 );
		printf( "0x%04X\r\n", dev.read_r16( AIO_STATUS ) );
		wait( 2 );
	}
}
