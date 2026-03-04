/** NXP Analog Front End class library for MCX
 *
 *  @author  Tedd OKANO
 *
 *  Copyright: 2026 Tedd OKANO
 *  Released under the MIT license
 */

#include "AFE_NXP.h"

#ifndef ARDUINO_AFE_NAFE33352_DRIVER_H
#define ARDUINO_AFE_NAFE33352_DRIVER_H

class NAFE33352_Base : public AFE_base
{
public:
	using	ch_setting_t	= uint16_t[ 4 ];
	// inherit reg templates from base for unqualified calls
	using AFE_base::reg;

	/** Register and Command definitions via AFE_Traits */
	using Register16	= AFE_Traits<NAFE33352_Base>::Register16;
	using Register24	= AFE_Traits<NAFE33352_Base>::Register24;
	using Command		= AFE_Traits<NAFE33352_Base>::Command;

	// Explicit overloads for this device's register types (after type aliases)
	inline auto reg(Register16 r)
	{
		return reg_read16(static_cast<uint16_t>(r));
	}

	inline auto reg(Register24 r)
	{
		return reg_read24(static_cast<uint16_t>(r));
	}

	template<typename V>
	inline void reg(Register16 r, V value)
	{
		static_assert(std::is_integral_v<V>, "value must be integral");
		reg_write16(static_cast<uint16_t>(r), static_cast<uint16_t>(value));
	}

	template<typename V>
	inline void reg(Register24 r, V value)
	{
		static_assert(std::is_integral_v<V>, "value must be integral");
		reg_write24(static_cast<uint16_t>(r), static_cast<uint32_t>(value));
	}

	/** Constructor to create a AFE_base instance */
	NAFE33352_Base( SPI& spi, bool spi_addr, bool highspeed_variant, int nINT, int DRDY, int SYN, int nRESET, int SYNCDAC );

	/** Destractor */
	virtual ~NAFE33352_Base();
	
	/** Set system-level config registers */
	virtual void boot( void );

	/** Issue RESET command */
	virtual void reset( bool hardware_reset = false );
	
	/** Configure logical channel
	 *
	 * @param ch logical channel number (0 ~ 15)
	 * @param cc0	16bit value to be set CH_CONFIG0 register (0x20)
	 * @param cc1	16bit value to be set CH_CONFIG1 register (0x21)
	 * @param cc2	16bit value to be set CH_CONFIG2 register (0x22)
	 * @param cc3	16bit value to be set CH_CONFIG3 register (0x23)
	 */
	virtual void open_logical_channel( int ch, uint16_t cc0, uint16_t cc1, uint16_t cc2, uint16_t dummy );

	/** Configure logical channel
	 *
	 * @param ch logical channel number (0 ~ 15)
	 * @param cc array for CH_CONFIG0, CH_CONFIG1, CH_CONFIG2 and CH_CONFIG3 values
	 */
	virtual void open_logical_channel( int ch, const uint16_t (&cc)[ 4 ] );

	class LogicalChannel : public LogicalChannel_Base
	{
	public:
		LogicalChannel();
		virtual ~LogicalChannel();
		
		void	configure( const uint16_t (&cc)[ 3 ] );
		void	configure( uint16_t cc0, uint16_t cc1 = 0x0000, uint16_t cc2 = 0x0000 );
	};
	
	LogicalChannel	logical_channel[ 16 ];

	class DAC
	{
	public:
		enum class ModeSelect : uint16_t {
			HI_Z		= 0,
			VOLTAGE,
			CURRENT,
			CURRENT_RECAL
		};

		DAC();
		virtual ~DAC();
		
		void	configure( const uint16_t (&cc)[ 6 ] );
		void	configure( uint16_t cc0, uint16_t cc1, uint16_t cc2, uint16_t cc3, uint16_t cc4, uint16_t cc5 );
		void	configure( ModeSelect mode, double full_scale_range = 0.00 );
		void 	configure( double full_scale_range );
		void	output( double value );
		DAC&	operator=( double value );
		
		NAFE33352_Base	*afe_ptr;
	private:
		ModeSelect		output_mode;
		double			full_scale;
	};
	
	DAC	dac;
	
private:	
	double 	calc_delay( int ch );
	void 	channel_info_update( uint16_t value );
	
public:
	void	open_dac_output( const uint16_t (&cc)[ 6 ] );


	/** Logical channel disable
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	virtual void close_logical_channel( int ch );

	/** All logical channel disable
	 */
	virtual void close_logical_channel( void );

	/** Logical channel enable
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	void	enable_logical_channel( int ch );

	/** Start ADC
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	virtual void start( int ch );

	/** Start ADC on all logical channel
	 */
	virtual void start( void );

	/** Start continuous AD conversion
	 */
	virtual void start_continuous_conversion();

	/** DRDY event select
	 *
	 * @param set true for DRDY by sequencer is done
	 */	
	virtual void DRDY_by_sequencer_done( bool flag = true );
	
	/** Read ADC for single channel
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	virtual raw_t	read( int ch );

	/** Read ADC for all channel
	 *
	 * @param data_ptr pointer to array to store ADC data
	 */
	virtual void	read( raw_t *data );

	/** Read ADC for all channel
	 *
	 * @param data_vctr vector object to store ADC data
	 */
	virtual void	read( std::vector<raw_t>& data_vctr );

	/** Read ADC for all channel
	 *
	 * @param data_ptr pointer to array to store ADC data
	 */
	virtual void	read( volt_t *data );

	/** Read ADC for all channel
	 *
	 * @param data_vctr vector object to store ADC data
	 */
	virtual void	read( std::vector<volt_t>& data_vctr );

	inline double raw2v( int ch, raw_t value )
	{
		if ( mux_setting[ ch ] == 13 )
			return	value - (double)(1 << 24) * 20.00 * 2.50 / (12.50 * (double)(1 << 24)) -1.50;
		else
			return	value * coeff_uV[ ch ];
	}
	
	/** DAC output
	 *
	 * @param data_vctr vector object to store ADC data
	 */
	virtual void	dac_out( double vi, double full_scale, uint8_t bit_length );
	int32_t			dac_code( double a, double full_scale, uint8_t bit_length );

	constexpr static double	pga_gain[]	= { 1.00, 16.00 };

	enum GainPGA : uint8_t {
		G_PGA_x_1_0,
		G_PGA_x16_0,
	};

	using	RegisterVariant	= std::variant<Register16, Register24>;
	using	RegVct			= std::vector<RegisterVariant>;

	// `reg_dump` implemented generically in AFE_base (template).

	// Non-template forwarding overloads so callers using braced-init-lists
	// (which convert to `RegVct`) continue to work without surprising
	// template-deduction failures.
	inline void reg_dump( RegVct reg_vctr ) { AFE_base::reg_dump<Register16,Register24>( reg_vctr ); }
	inline void reg_dump( Register24 addr, int length ) { AFE_base::reg_dump<Register24>( addr, length ); }
	void	logical_ch_config_view( void );
		
	/** Command
	 *	
	 * @param com "Comand" type or uint16_t value
	 */
	virtual void		command( uint16_t com );

	/* Register accessors are provided by templated `reg()` in AFE_base (AFE_NXP.h)
	 * No virtual declarations here to avoid duplicate/undefined symbols.
	 */
	
	/** Register bit operation
	 *
	 *	overwrite bits i a register
	 * @param reg register specified by Register16 or Register24 member
	 * @param mask mask bits
	 * @param reg value to over write
	 */
	template<typename T>
	uint32_t	bit_op( T rg, uint32_t mask, uint32_t value )
	{
		uint32_t v	= reg( rg );

		v	&= mask;
		v	|= value & ~mask;

		reg( rg, v );
		
		return v;
	}
	
	/** Read part_number
	 *
	 * @return 0x13388B40 
	 */
	uint64_t	part_number( void );

	/** Read rivision number
	 *
	 * @return PN0 register value & 0xF
	 */
	uint8_t	revision_number( void );

	/** Read serial number
	 *
	 * @return serial number
	 */
	uint64_t	serial_number( void );

	/** Die temperature
	 *
	 * @return die temperature in celsius
	 */
	float	temperature( void );
};

class NAFE33352 : public NAFE33352_Base
{
public:	
	/** Constructor to create a NAFE33352 instance */
	NAFE33352( SPI& spi, bool spi_addr = 0, bool highspeed_variant = false, int nINT = D7, int DRDY = D4, int SYN = D2, int nRESET = D5, int SYNCDAC = D3 );

	/** Destractor */
	virtual ~NAFE33352();
};

class NAFE33352_UIOM : public NAFE33352_Base
{
public:	
	/** Constructor to create a NAFE33352 instance */
	NAFE33352_UIOM( SPI& spi, bool spi_addr = 0, bool highspeed_variant = false, int nINT = D7, int DRDY = D4, int SYN = D2, int nRESET = DISABLED_PIN, int SYNCDAC = D3 );

	/** Destractor */
	virtual ~NAFE33352_UIOM();
};


inline NAFE33352_Base::Register16 operator+( NAFE33352_Base::Register16 rn, int n )
{
	return static_cast<NAFE33352_Base::Register16>( static_cast<uint16_t>( rn ) + n );
}

inline NAFE33352_Base::Register16 operator+( int n, NAFE33352_Base::Register16 rn )
{
	return static_cast<NAFE33352_Base::Register16>( n + static_cast<uint16_t>( rn ) );
}

inline NAFE33352_Base::Register24 operator+( NAFE33352_Base::Register24 rn, int n )
{
	return static_cast<NAFE33352_Base::Register24>( static_cast<uint16_t>( rn ) + n );
}

inline NAFE33352_Base::Register24 operator+( int n, NAFE33352_Base::Register24 rn )
{
	return static_cast<NAFE33352_Base::Register24>( n + static_cast<uint16_t>( rn ) );
}

#endif // !ARDUINO_AFE_NAFE33352_DRIVER_H
