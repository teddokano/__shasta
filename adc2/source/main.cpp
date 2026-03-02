#include	"r01lib.h"
#include	"afe/NAFE33352.h"

SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
NAFE33352		shasta( spi, 0 );

InterruptIn		d4( D4 );

using enum NAFE33352::Register16;
using enum NAFE33352::Register24;
using enum NAFE33352::Command;

constexpr double	ref_point	=  20.00;

int32_t	mA2DacCode( double value )
{
	return	~(int32_t)((double)0x614780 * value / ref_point);
}

void set_current( double c )
{
//	shasta.reg( AO_DATA, mA2DacCode( c ) );

	wait( 0.2 );
//	printf( "now, output is %+4.1lfmA, status = 0x%04X\r\n", c, shasta.reg( AIO_STATUS ) );
}

int main( void )
{
//	NAFE33352		shasta( spi, 0, false, D2, D4 );

	printf( "***** Hello, SHASTA board! *****\r\n" );

	spi.frequency( 1'000'000 );
	spi.mode( 1 );

	shasta.begin();
//	shasta.set_DRDY_callback( cb );

#if 1
	printf( "Part number          = 0x%04X%04X%02\r\n", shasta.reg( PN2 ), shasta.reg( PN1 ), shasta.reg( PN0_REV ) >> 8 );
	printf( "Revision             = 0x%02X\r\n", shasta.reg( PN0_REV ) & 0xFF );
	printf( "Unique serial number = 0x%06X%06X\r\n", shasta.reg( SERIAL1 ), shasta.reg( SERIAL0 ) );

	printf( "GAINCOEF5 = 0x%lX\r\n", shasta.reg( GAIN_COEF5 )  );
	printf( "GAINCOEF5 = %lf\r\n",   shasta.reg( GAIN_COEF5 ) / (double)0x400000  );
#endif

//	shasta.dac.configure( 0x6061, 0x1000, 0x87FF, 0x8200, 0xE7FF, 0x0C00 );
	shasta.dac.configure( 0x6041, 0x1000, 0x87FF, 0x8200, 0xE7FF, 0x0C00 );
	set_current( 2.0 );

	shasta.logical_channel[  0 ].configure( 0x0020, 0x50B4, 0x5000 );
	shasta.logical_channel[  1 ].configure( 0x0080, 0x5064, 0x5000 );
	shasta.logical_channel[  2 ].configure( 0x0088, 0x5064, 0x5000 );

//	shasta.use_DRDY_trigger( false );	//	default = true

	double	data;

	while ( true )
	{
		data	= shasta.logical_channel[ 0 ] * 1e-6;
		printf( "    %lfV", data );

		data	= shasta.logical_channel[ 1 ] * 1e-6;
		printf( "    %lfV", data );

		data	= shasta.logical_channel[ 2 ] * 1e-6;
		printf( "    %lfV", data );

		printf( "\r\n" );

		wait( 1.0 );
	}
}
