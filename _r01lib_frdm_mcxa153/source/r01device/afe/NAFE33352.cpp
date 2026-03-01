/** NXP Analog Front End class library for MCX
 *
 *  @author  Tedd OKANO
 *
 *  Copyright: 2026 Tedd OKANO
 *  Released under the MIT license
 */

#include	"NAFE33352.h"
#include	"r01lib.h"
#include	<math.h>

using enum	NAFE33352_Base::Register16;
using enum	NAFE33352_Base::Register24;
using enum	NAFE33352_Base::Command;


NAFE33352_Base::LogicalChannel::LogicalChannel() : LogicalChannel_Base()
{
}

NAFE33352_Base::LogicalChannel::~LogicalChannel()
{
	disable();
}

void NAFE33352_Base::LogicalChannel::configure( const uint16_t (&cc)[ 3 ] )
{
	const uint16_t	tmp_ch_config[ 4 ]	= { cc[ 0 ], cc[ 1 ], cc[ 2 ], 0 };
	afe_ptr->open_logical_channel( ch_number, tmp_ch_config );
}

void NAFE33352_Base::LogicalChannel::configure( uint16_t cc0, uint16_t cc1, uint16_t cc2 )
{
	const uint16_t	tmp_ch_config[ 4 ]	= { cc0, cc1, cc2, 0 };
	afe_ptr->open_logical_channel( ch_number, tmp_ch_config );
}

/* NAFE33352_Base class ******************************************/

NAFE33352_Base::NAFE33352_Base( SPI& spi, bool spi_addr, bool hsv, int nINT, int DRDY, int SYN, int nRESET ) 
	: AFE_base( spi, spi_addr, hsv, nINT, DRDY, SYN, nRESET )
{
	for ( auto i = 0; i < 16; i++ )
	{
		logical_channel[ i ].afe_ptr	= this;
		logical_channel[ i ].ch_number	= i;
	}
}

NAFE33352_Base::~NAFE33352_Base()
{
}

void NAFE33352_Base::boot( void )
{
	command( CMD_ADC_ABORT );
	command( CMD_AO_ABORT );
	wait( 0.001 );
	
	DRDY_by_sequencer_done( true );
}

void NAFE33352_Base::reset( bool hardware_reset )
{
	if ( hardware_reset )
	{
		pin_nRESET	= 0;
		wait( 0.001 );
		pin_nRESET	= 1;
	}
	else
	{
		command( CMD_RESET ); 
	}
	
	constexpr uint16_t	CHIP_READY	= 1 << 13;
	constexpr auto		RETRY		= 10;
	
	for ( auto i = 0; i < RETRY; i++ )
	{
		wait( 0.003 );
		if ( reg( SYS_STATUS ) & CHIP_READY )
			return;
	}
	
	panic( "NAFE13388 couldn't get ready. Check power supply or pin conections\r\n" );
}

void NAFE33352_Base::open_logical_channel( int ch, const uint16_t (&cc)[ 4 ] )
{	
	constexpr double	pow2_24	= (double)(1 << 24);
	double				coeff	= 0.00;
	command( ch );

	switch ( (cc[ 0 ] >> 3) & 0x1F )
	{
		case 0:
		case 3:
			coeff	= (20.00 * 2.50) / (12.5 * pow2_24);
			break;
		case 1:
			coeff	= (20.00 * 2.50) / ((cc[ 0 ] & 0x0200 ? 16.00 : 1.00) * pow2_24);
			break;
		case 2:
		case 7:
			coeff	= (20.00 * 2.50) / pow2_24;
			break;
		case 4:
			coeff	= (20.00 * 2.50) / ((cc[ 0 ] & 0x0200 ? 16.00 : 1.00) * pow2_24);
			break;
		case 5:
			coeff	= (20.00 * 2.50) / ((cc[ 0 ] & 0x0100 ? 16.00 : 1.00) * pow2_24);
			break;
		case 6:
			coeff	= (20.00 * 2.50) / (3.7989 * pow2_24);
			break;
		case 8:
			coeff	= (20.00 * 2.50) / (2.50 * pow2_24);
			break;
		case 9:
		case 10:
		case 11:
		case 15:
		case 18:
			coeff	= (20.00 * 2.50) / (12.5 * pow2_24) + 1.50;
			break;
		case 12:
			coeff	= (20.00 * 2.50) / (12.5 * pow2_24) - 1.50;
			break;
		case 13:
			//coeff	= (20.00 * 2.50) / (12.5 * pow2_24) - 1.50;
			break;
		case 14:
			coeff	= (2.00 * 20.00 * 2.50) / (12.5 * pow2_24) - 1.50;
			break;
		case 16:
		case 17:
			coeff	= (40.00 * 20.00 * 2.50) / (12.5 * pow2_24) - 1.50;
			break;
	}
	
	coeff_uV[ ch ]		= coeff * 1e6;
	
	for ( auto i = 0; i < 3; i++ )
		reg( AI_CONFIG0 + i, cc[ i ] );
	
	enable_logical_channel( ch );
	
	ch_delay[ ch ]		= calc_delay( ch );
}

void NAFE33352_Base::channel_info_update( uint16_t value )
{
	constexpr auto	bit_length	= 16;
	enabled_channels			= 0;
	total_delay					= 0.00;
	
	memset( sequence_order, 0, 16 );
		
	for ( auto i = 0; i < bit_length; i++ )
	{
		if ( value & (0x1 << i) )
		{
			sequence_order[ enabled_channels ]	= i;
			enabled_channels++;
			total_delay	+= ch_delay[ i ];
		}
	}

#if 0
	for ( auto i = 0; i < bit_length; i++ )
		printf( " %x", sequence_order[ i ] );
	printf( "\r\n" );
#endif
}

double NAFE33352_Base::calc_delay( int ch )
{
	constexpr static double	data_rates[]	= {	   288000, 192000, 144000, 96000, 72000, 48000, 36000, 24000, 
													18000,  12000,   9000,  6000,  4500,  3000,  2250,  1125, 
													 562.5,    400,    300,   200,   100,    60,    50,    30, 
														25,     20,     15,    10,   7.5, 						};
	constexpr static uint16_t	delays[]	= {		0,   2,   4,   6,   8,  10,   12,  14, 
												   16,  18,  20,  28,  38,  40,   42,  56, 
												   64,  76,  90, 128, 154, 178, 204, 224, 
												  256, 358, 512, 716, 
												  1024, 1664, 3276, 7680, 19200, 23040, };
	
	command( ch );

	uint16_t ch_config1	= reg( AI_CONFIG1 );
	uint16_t ch_config2	= reg( AI_CONFIG2 );
	
	uint8_t		adc_data_rate		= (ch_config1 >>  3) & 0x001F;
	uint8_t		adc_sinc			= (ch_config1 >>  0) & 0x0007;
	uint8_t		ch_delay			= (ch_config2 >> 10) & 0x003F;
	bool		adc_normal_setting	= (ch_config2 >>  9) & 0x0001;
	bool		ch_chop				= (ch_config2 >>  7) & 0x0001;
	
	double		base_freq			= data_rates[ adc_data_rate ];
	double		delay_setting		= delays[ ch_delay ] / 4608000.00;
	
	if ( highspeed_variant )
	{
		base_freq		*= 2.00;
		delay_setting	/= 2.00;		
	}
	
	if ( (28 < adc_data_rate) || (4 < adc_sinc) || ((adc_data_rate < 12) && (adc_sinc)) )
		return 0.00;
	
	if ( !adc_normal_setting  )
		base_freq	/= (adc_sinc + 1);
	
	if ( ch_chop )
		base_freq	/= 2;
	
#if 0
	printf( "base_freq = %lf\r\n", base_freq );
	printf( "delay_setting = %lf\r\n", delay_setting  );
	printf( "channel delay = %lf\r\n", (1 / base_freq) + delay_setting  );
#endif
	
	return (1 / base_freq) + delay_setting;
}


void NAFE33352_Base::open_logical_channel( int ch, uint16_t cc0, uint16_t cc1, uint16_t cc2, uint16_t dummy )
{	
	const ch_setting_t	tmp_ch_config	= { cc0, cc1, cc2 };
	open_logical_channel( ch, tmp_ch_config );
}

void NAFE33352_Base::enable_logical_channel( int ch )
{	
	const uint16_t	setbit	= 0x1 << (ch + 8);
	const uint16_t	bits	= bit_op( AI_MULTI_CH_EN, ~setbit, setbit );

	channel_info_update( bits >> 8 );
}

void NAFE33352_Base::close_logical_channel( int ch )
{	
	const uint16_t	clearingbit	= 0x1 << (ch + 8);
	const uint16_t	bits		= bit_op( AI_MULTI_CH_EN, ~clearingbit, ~clearingbit );

	channel_info_update( bits >> 8 );
}

void NAFE33352_Base::close_logical_channel( void )
{	
	reg( AI_MULTI_CH_EN, 0x0000 );
	channel_info_update( 0x0000 );
}

void NAFE33352_Base::start( int ch )
{
	command( ch     );
	command( CMD_SS );
}

void NAFE33352_Base::start( void )
{
	command( CMD_MM );
}

void NAFE33352_Base::start_continuous_conversion( void )
{
	command( CMD_MC );
}

void NAFE33352_Base::DRDY_by_sequencer_done( bool flag )
{
	bit_op( AI_SYSCFG, ~0x0100, flag ? 0x0100 : 0x0000 );	
}

int32_t NAFE33352_Base::read( int ch )
{
	return reg( AI_DATA0 + ch );
}

void NAFE33352_Base::read( raw_t *data )
{
	burst( (uint32_t *)data, enabled_channels );
}

void NAFE33352_Base::read( std::vector<raw_t>& data_vctr )
{
	raw_t	raw_data[ 16 ];
	
	read( raw_data );
	std::copy( raw_data, raw_data + enabled_channels, data_vctr.begin() );
}

void NAFE33352_Base::read( microvolt_t *data )
{
	raw_t	raw_data[ 16 ];
	
	read( raw_data );
	
	for ( auto i = 0; i < enabled_channels; i++ )
		data[ i ]	= raw2uv( sequence_order[ i ], raw_data[ i ] );
}

void NAFE33352_Base::read( std::vector<microvolt_t>& data_vctr )
{
	raw_t	raw_data[ 16 ];
	
	read( raw_data );
	
	for ( auto i = 0; i < enabled_channels; i++ )
		data_vctr[ i ]	= raw2uv( sequence_order[ i ], raw_data[ i ] );
}

void NAFE33352_Base::command( uint16_t com )
{
	write_r16( com );
}

void NAFE33352_Base::reg( Register16 r, uint16_t value )
{
	write_r16( static_cast<uint16_t>( r ), value );
}

void NAFE33352_Base::reg( Register24 r, uint32_t value )
{
	write_r24( static_cast<uint16_t>( r ), value );
}

uint16_t NAFE33352_Base::reg( Register16 r )
{
	return read_r16( static_cast<uint16_t>( r ) );
}

uint32_t NAFE33352_Base::reg( Register24 r )
{
	return read_r24( static_cast<uint16_t>( r ) );
}

uint64_t NAFE33352_Base::part_number( void )
{
	return (static_cast<uint64_t>( reg( PN2 ) ) << (16 + 8)) | static_cast<uint64_t>( reg( PN1 ) ) << 8 | reg( PN0_REV ) >> 8;
}

uint8_t NAFE33352_Base::revision_number( void )
{
	return reg( PN0_REV ) & 0xF;
}

uint64_t NAFE33352_Base::serial_number( void )
{
	uint64_t	serial_number;

	serial_number	  = reg( SERIAL1 );
	serial_number	<<=  24;
	return serial_number | reg( SERIAL0 );
}
			
float NAFE33352_Base::temperature( void )
{
	return reg( DIE_TEMP ) / 64.0;
}




/* NAFE13388 class ******************************************/

NAFE33352::NAFE33352( SPI& spi, bool spi_addr, bool hsv, int nINT, int DRDY, int SYN, int nRESET ) 
	: NAFE33352_Base( spi, spi_addr, hsv, nINT, DRDY, SYN, nRESET )
{
}

NAFE33352::~NAFE33352()
{
}

//double	NAFE13388::coeff_uV[ 16 ];
