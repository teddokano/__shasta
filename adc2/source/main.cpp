#include	"r01lib.h"
#include	"afe/NAFE33352_UIOM.h"

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
NAFE33352_UIOM	shasta( spi, 0 );

using enum NAFE33352_UIOM::Register16;
using enum NAFE33352_UIOM::Register24;
using enum NAFE33352_UIOM::Command;
using enum NAFE33352_UIOM::DAC::ModeSelect;

constexpr double	ref_point	=  20.00;

int main( void )
{
	printf( "***** Hello, SHASTA board! *****\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	shasta.begin();

#if 1
	printf( "Part number          = 0x%04X%04X%02\r\n", shasta.reg( PN2 ), shasta.reg( PN1 ), shasta.reg( PN0_REV ) >> 8 );
	printf( "Revision             = 0x%02X\r\n", shasta.reg( PN0_REV ) & 0xFF );
	printf( "Unique serial number = 0x%06X%06X\r\n", shasta.reg( SERIAL1 ), shasta.reg( SERIAL0 ) );

	printf( "GAINCOEF5 = 0x%lX\r\n", shasta.reg( GAIN_COEF5 )  );
	printf( "GAINCOEF5 = %lf\r\n",   shasta.reg( GAIN_COEF5 ) / (double)0x400000  );
#endif

#if 0
	shasta.dac.configure( 0x6061, 0x1000, 0x87FF, 0x8200, 0xE7FF, 0x0C00 );
	shasta.dac.mode( CURRENT );
	shasta.dac.output( 0.02 );
#else
	shasta.dac.configure( 0x6041, 0x1000, 0x87FF, 0x8200, 0xE7FF, 0x0C00 );
	shasta.dac.mode( VOLTAGE );
	shasta.dac	= 5.00;
#endif
	shasta.logical_channel[  0 ].configure( 0x0020, 0x50B4, 0x5000 );
	shasta.logical_channel[  1 ].configure( 0x0080, 0x5064, 0x5000 );
	shasta.logical_channel[  2 ].configure( 0x0088, 0x5064, 0x5000 );
	shasta.logical_channel[  3 ].configure( 0x0038, 0x2064, 0x5000 );

//	shasta.use_DRDY_trigger( false );	//	default = true

	double	data;

	while ( true )
	{
		for ( auto i = 0; i < 4; i++ )
		{
			data	= shasta.logical_channel[ i ] * 1e-6;
			printf( "    %lfV", data );
		}

		printf( "    AIO_STATUS = 0x%04X\r\n", shasta.reg( AIO_STATUS ) );

		wait( 1.0 );
	}
}
