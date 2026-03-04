/** NXP Analog Front End class library for MCX
 *
 *  @class   NAFE13388
 *  @author  Tedd OKANO
 *
 *  Copyright: 2023 - 2026 Tedd OKANO
 *  Released under the MIT license
 *
 *  A simple class library for NXP Analog Front End: NAFE13388
 *
 *  Example:
 *  @code
 *  #include	"r01lib.h"
 *  #include	"afe/NAFE13388_UIM.h"
 *  
 *  using	volt_t	= NAFE13388_UIM::volt_t;
 *  
 *   SPI				spi( ARD_MOSI, ARD_MISO, ARD_SCK, ARD_CS );	//	MOSI, MISO, SCLK, CS
 *   NAFE13388_UIM	afe( spi );
 *  
 *   int main( void )
 *   {
 *  	 printf( "***** Hello, NAFE13388 UIM board! *****\r\n" );
 *  
 *  	 spi.frequency( 1'000'000 );
 *  	 spi.mode( 1 );
 *  
 *  	 afe.begin();
 *  
 *  	 afe.logical_channel[ 0 ].configure( 0x1710, 0x00A4, 0xBC00, 0x0000 );
 *  	 afe.logical_channel[ 1 ].configure( 0x2710, 0x00A4, 0xBC00, 0x0000 );
 *  
 *  	 afe.use_DRDY_trigger( false );	//	default = true
 *  
 *  	 printf( "\r\nenabled logical channel(s) %2d\r\n", afe.enabled_logical_channels() );
 *  
 *  	 volt_t	data0;
 *  	 volt_t	data1;
 *  
 *  	 while ( true )
 *  	 {
 *  		 data0	= afe.logical_channel[ 0 ];	//	read logical channel 0
 *  		 data1	= afe.logical_channel[ 1 ];	//	read logical channel 1
 *  
 *  		 printf( "   channel 0 : %12.9lfV,   channel 1 : %12.9lfV\r\n", data0 * 1e-6, data1 * 1e-6 );
 *  	 }
 *   }
 *  @endcode
 */

#ifndef ARDUINO_AFE_DRIVER_H
#define ARDUINO_AFE_DRIVER_H

#include	<stdint.h>
#include	"r01lib.h"
#include	"SPI_for_AFE.h"
#include	"AFE_Traits.h"
#include	<cmath>
#include	<vector>
#include	<variant>
#include	<algorithm>
#include	<functional>

#define		NON_TEMPLATE_VERSION_FOR_START_AND_READ

class AFE_base : public SPI_for_AFE
{
public:
	/** ADC readout types */
	using raw_t		= int32_t;
	using volt_t	= double;
	using ampere_t	= double;

	/** Constructor to create a AFE_base instance */
	AFE_base( SPI& spi, bool spi_addr, bool highspeed_variant, int nINT, int DRDY, int SYN, int nRESET, int SYNCDAC  );

	/** Destractor */
	virtual ~AFE_base();
	
	/** Begin the device operation
	 *
	 *	NAFE13388 initialization. It does following steps
	 *	(1) Call reset()
	 *	(2) Call boot()
	 */
	virtual void begin( void );

	/** Set system-level config registers */
	virtual void boot( void )	= 0;

	/** Issue RESET command */
	virtual void reset( bool hardware_reset = false )	= 0;
	
	/** set callback function when DRDY comes */
	using	callback_fp_t	= std::function<void(void)>;
	virtual void set_DRDY_callback( callback_fp_t fnc );
	
	/** Configure logical channel
	 *
	 * @param ch logical channel number (0 ~ 15)
	 * @param cc0	16bit value to be set CH_CONFIG0 register (0x20)
	 * @param cc1	16bit value to be set CH_CONFIG1 register (0x21)
	 * @param cc2	16bit value to be set CH_CONFIG2 register (0x22)
	 * @param cc3	16bit value to be set CH_CONFIG3 register (0x23)
	 */
	virtual void open_logical_channel( int ch, uint16_t cc0, uint16_t cc1, uint16_t cc2, uint16_t cc3 )	= 0;

	/** Configure logical channel
	 *
	 * @param ch logical channel number (0 ~ 15)
	 * @param cc array for CH_CONFIG0, CH_CONFIG1, CH_CONFIG2 and CH_CONFIG3 values
	 */
	virtual void open_logical_channel( int ch, const uint16_t (&cc)[ 4 ] )	= 0;

	/** Logical channel disable
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	virtual void close_logical_channel( int ch )		= 0;

	/** All logical channel disable
	 */
	virtual void close_logical_channel( void )			= 0;

	/** Logical channel enable
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	virtual void enable_logical_channel( int ch )		= 0;

	/** Start ADC
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	virtual void start( int ch )						= 0;

	/** Start ADC on all logical channel
	 */
	virtual void start( void )							= 0;

	/** Start continuous AD conversion
	 */
	virtual void start_continuous_conversion( void )	= 0;

	/** DRDY event select
	 *
	 * @param set true for DRDY by sequencer is done
	 */	
	virtual void DRDY_by_sequencer_done( bool flag = true )	= 0;

	/** Read ADC for single channel
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	virtual raw_t	read( int ch )							= 0;

	/** Read ADC for all channel
	 *
	 * @param data_ptr pointer to array to store ADC data
	 */
	virtual void	read( raw_t *data_ptr )					= 0;

	/** Read ADC for all channel
	 *
	 * @param data_vctr vector object to store ADC data
	 */
	virtual void	read( std::vector<raw_t>& data_vctr )	= 0;

	/** Start and read ADC for single  channel
	 *
	 * @param ch logical channel number (0 ~ 15)
	 */
	virtual raw_t	start_and_read( int ch );

	/** Low-level register helpers (implemented in AFE_NXP.cpp)
	 *  These provide a single implementation point for register access
	 *  and are used by device-specific `reg()` wrappers.
	 */
	void	reg_write16(uint16_t addr, uint16_t value);
	void	reg_write24(uint16_t addr, uint32_t value);
	uint16_t	reg_read16(uint16_t addr);
	uint32_t	reg_read24(uint16_t addr);

	template<typename Reg16T, typename Reg24T>
	inline void reg_dump(const std::vector<std::variant<Reg16T, Reg24T>>& reg_vctr)
	{
		table_view( reg_vctr.size(), 4,
			[&reg_vctr, this]( int v )
			{
				if ( const Reg24T *ap = std::get_if<Reg24T>( &reg_vctr[ v ] ) )
					printf( "    0x%04X: 0x%06lX", static_cast<int>( *ap ), (unsigned long)( this->reg( *ap ) & 0xFFFFFF ) );
				else if ( const Reg16T *ap = std::get_if<Reg16T>( &reg_vctr[ v ] ) )
					printf( "    0x%04X: 0x  %04X", static_cast<int>( *ap ), (unsigned int)( this->reg( *ap ) & 0xFFFF ) );
			},
			[](){ printf( "\r\n" ); }
		);
	}

	template<typename Reg24T>
	inline void reg_dump( Reg24T addr, int length )
	{
		table_view( length, 4, [this, addr]( int v ){ printf( "  %8lu @ 0x%04X", (unsigned long)this->reg( v + addr ), v + (uint16_t)addr ); }, [](){ printf( "\r\n" ); } );
	}

	template<typename Reg16T, typename Reg24T>
	inline void reg_dump( std::initializer_list<std::variant<Reg16T, Reg24T>> init )
	{
		std::vector<std::variant<Reg16T, Reg24T>> v( init );
		reg_dump<Reg16T, Reg24T>( v );
	}
	
#ifdef	NON_TEMPLATE_VERSION_FOR_START_AND_READ

	/** Start and read ADC for all channel
	 *
	 * @param data_ptr pointer to array to store ADC data
	 */
	virtual void	start_and_read( raw_t *data_ptr );

	/** Start and read ADC for all channel
	 *
	 * @param data_vctr vector object to store ADC data
	 */
	virtual void	start_and_read( std::vector<raw_t>& data_vctr );
#else
	template<typename T>
	inline void start_and_read( T data )
	{
		double	wait_time	= cbf_DRDY ? -1.0 : total_delay * delay_accuracy;
		
		start();
		wait_conversion_complete( wait_time );
		
		read( data );
	};
#endif

	enum LV_mux_sel : uint8_t {
		REF2_REF2	= 0,
		GPIO0_GPIO1,
		REFCOARSE_REF2,
		VADD_REF2,
		VHDD_REF2,
		REF2_VHSS,
		HV_MUX,
	};

	
	/** Convert raw output to micro-volt
	 *
	 * @param ch logical channel number to select its gain coefficient
	 * @param value ADC read value
	 */
	inline double raw2uv( int ch, raw_t value )
	{
		return raw2v( ch, value ) * 1e6;
	}
	
	/** Convert raw output to milli-volt
	 *
	 * @param ch logical channel number to select its gain coefficient
	 * @param value ADC read value
	 */
	inline double raw2mv( int ch, raw_t value )
	{
		return raw2v( ch, value ) * 1e3;
	}
	
	/** Convert raw output to volt
	 *
	 * @param ch logical channel number to select its gain coefficient
	 * @param value ADC read value
	 */
	virtual double raw2v( int ch, raw_t value )	= 0;
	
	/** Coefficient to convert from ADC read value to micro-volt
	 *
	 * @param ch logical channel number
	 */
	inline double coeff_mV( int ch )
	{
		return coeff_uV[ ch ];
	}
	
	/** Caliculated delay from logical channel setting (for single channel)
	 *
	 * @param ch logical channel number
	 */
	inline double drdy_delay( int ch )
	{
		return ch_delay[ ch ];
	}

	/** Caliculated delay from logical channel setting (for all channels)
	 */
	inline double drdy_delay( void )
	{
		return total_delay;
	}

	/** Number of enabled logical channels */
	inline int enabled_logical_channels( void )
	{
		return enabled_channels;
	}
	
	/** Switch to use DRDY to start ADC result reading
	 *
	 * @param use true (default) to use DRDY. if false, caliculated delay is used to start reading. 
	 */
	void	use_DRDY_trigger( bool use = true );

protected:
	bool			highspeed_variant;
	InterruptIn		pin_nINT;
	InterruptIn		pin_DRDY;
	DigitalOut		pin_SYN;
	DigitalOut		pin_nRESET;
	DigitalOut		pin_SYNCDAC;

	int 			bit_count( uint32_t value );
	void			table_view( int size, int cols, std::function<void(int)> view, std::function<void(void)> linefeed = nullptr );

	/** Number of enabled logical channels */
	int				enabled_channels;
	
	/** Number of enabled logical channels */
	uint8_t			sequence_order[ 16 ];
	
	/** Coefficient to convert from ADC read value to micro-volt */
	double			coeff_uV[ 16 ];

	/** Multiplexer setting */
	int				mux_setting[ 16 ];

	
	/** Channel delay */
	double			ch_delay[ 16 ];
	double			total_delay;
	static double	delay_accuracy;
	

	uint32_t		drdy_count;
	volatile bool	drdy_flag;

	constexpr static uint32_t	timeout_limit	= 100000000;

	static callback_fp_t	cbf_DRDY;
public:
	virtual void			init( void );
protected:
	void					default_drdy_cb( void );
	
	static void				DRDY_cb( void );
	int						wait_conversion_complete( double delay = -1.0 );

	template< typename R, typename V >
	inline void reg( R r, V value )
	{
		using underlying	= std::underlying_type_t<R>;
		
		if ( std::is_same_v<underlying, uint16_t> )
		{
			reg_write16( static_cast<uint16_t>( r ), static_cast<uint16_t>( value ));
		}
		else
		{
			reg_write24( static_cast<uint16_t>( r ), value );
		}
	}

	// unified read: return width deduced from register type
	template< typename R >
	inline uint32_t reg( R r )
	{
		using underlying	= std::underlying_type_t<R>;
		
		if ( std::is_same_v<underlying, uint16_t> )
		{
			return static_cast<uint32_t>( reg_read16( static_cast<uint16_t>( r ) ) );
		}
		else
		{
			return reg_read24( static_cast<uint16_t>( r ) );
		}
	}
};

class LogicalChannel_Base
{
public:
	LogicalChannel_Base() {}
	virtual ~LogicalChannel_Base() {}
	
	void	enable( void );
	void	disable( void );

	template<class T> T read(void);
		
	operator AFE_base::raw_t(void);
	operator AFE_base::volt_t(void);

	template<class T> double operator+( T v ) { return (double)(*this) + (double)v; }
	template<class T> double operator-( T v ) { return (double)(*this) - (double)v; }
	template<class T> double operator*( T v ) { return (double)(*this) * (double)v; }
	template<class T> double operator/( T v ) { return (double)(*this) / (double)v; }
	template<class T> friend double operator+( T v, LogicalChannel_Base lc ) { return (double)v + (double)lc; }
	template<class T> friend double operator-( T v, LogicalChannel_Base lc ) { return (double)v - (double)lc; }
	template<class T> friend double operator*( T v, LogicalChannel_Base lc ) { return (double)v * (double)lc; }
	template<class T> friend double operator/( T v, LogicalChannel_Base lc ) { return (double)v / (double)lc; }
	
	int			ch_number;
	AFE_base	*afe_ptr;
};

class NAFE13388_Base : public AFE_base
{
public:
	using	ch_setting_t	= uint16_t[ 4 ];
	
	/** Register and Command definitions via AFE_Traits */
	using Register16	= AFE_Traits<NAFE13388_Base>::Register16;
	using Register24	= AFE_Traits<NAFE13388_Base>::Register24;
	using Command		= AFE_Traits<NAFE13388_Base>::Command;

	typedef struct	_reference_point	{
		double	voltage;
		int32_t	data;
	} reference_point;

	typedef struct	_ref_points	{
		int				coeff_index;
		reference_point	high;
		reference_point	low;
		int				cal_index;
	} ref_points;
	
	/** Constructor to create a AFE_base instance */
	NAFE13388_Base( SPI& spi, bool spi_addr, bool highspeed_variant, int nINT, int DRDY, int SYN, int nRESET );

	/** Destractor */
	virtual ~NAFE13388_Base();
	
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
	virtual void open_logical_channel( int ch, uint16_t cc0, uint16_t cc1, uint16_t cc2, uint16_t cc3 );

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
		
		void	configure( const uint16_t (&cc)[ 4 ] );
		void	configure( uint16_t cc0 = 0x0000, uint16_t cc1 = 0x0000, uint16_t cc2 = 0x0000, uint16_t cc3 = 0x0000 );
	};
	
	LogicalChannel	logical_channel[ 16 ];

private:	
	double 	calc_delay( int ch );
	void 	channel_info_update( uint16_t value );
public:
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
		double	v	= value * coeff_uV[ ch ];

		if ( HV_MUX != mux_setting[ ch ] )
		{
			switch ( mux_setting[ ch ] )
			{
				case REF2_REF2:
				case GPIO0_GPIO1:
					return v;
					break;
				case REFCOARSE_REF2:
				case VADD_REF2:
					return 2.00 * (v + 1.50);
					break;
				case VHDD_REF2:
					return 32.00 * (v + 0.25);
					break;
				case REF2_VHSS:
					return -32.00 * (v - 0.25);
					break;
			}
		}		
		return v;
	}
	
	constexpr static double	pga_gain[]	= { 0.2, 0.4, 0.8, 1, 2, 4, 8, 16 };

	enum GainPGA : uint8_t {
		G_PGA_x_0_2	= 0,
		G_PGA_x_0_4,
		G_PGA_x_0_8,
		G_PGA_x_1_0,
		G_PGA_x_2_0,
		G_PGA_x_4_0,
		G_PGA_x_8_0,
		G_PGA_x16_0,
	};

	using	RegisterVariant	= std::variant<Register16, Register24>;

	using	RegVct			= std::vector<RegisterVariant>;

	// `reg_dump` is provided by AFE_base as a template to avoid
	// duplicate implementations in each derived device class.
	
	// Forwarding overloads for convenience and to preserve existing callsites
	// that pass a braced-init-list which converts to `RegVct`.
	inline void reg_dump( RegVct reg_vctr ) { AFE_base::reg_dump<Register16,Register24>( reg_vctr ); }
	inline void reg_dump( Register24 addr, int length ) { AFE_base::reg_dump<Register24>( addr, length ); }

	void	logical_ch_config_view( void );
		
	/** Command
	 *	
	 * @param com "Comand" type or uint16_t value
	 */
	virtual void		command( uint16_t com );

	// bring `reg` templates from base into this class's lookup scope
	using AFE_base::reg;

	// Explicit overloads for this device's register types (after type aliases)
	inline auto reg( Register16 r )
	{
		return reg_read16( static_cast<uint16_t>( r ) );
	}

	inline auto reg( Register24 r )
	{
		return reg_read24( static_cast<uint16_t>( r ) );
	}

	template<typename V>
	inline void reg( Register16 r, V value )
	{
		reg_write16( static_cast<uint16_t>(r), static_cast<uint16_t>( value ) );
	}

	template<typename V>
	inline void reg( Register24 r, V value )
	{
		reg_write24( static_cast<uint16_t>(r), static_cast<uint32_t>( value ) );
	}
	
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
	uint32_t	part_number( void );

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
	
	/** Gain and offset coefficient customization
	 *
	 *	Sets gain and offset coefficients with given target ADC read-out values at two reference voltaeg points
	 * @param ref struct to define the target coefficient index and two reference poins and reference pre-calibrated coeffs
	 */
	void	gain_offset_coeff( const ref_points &ref );

	enum CalibrationError : int {
		NoError		=  0,
		GainError	= -1,
		OffsetError	= -2,
	};
	
	/** On-board calibration with specified input and voltage
	 *
	 *	Updates coefficients at pga_gain_index
	 *	
	 * @param pga_gain_index			PGA gain index to measure and update the coefficients
	 * @param channel_selection			Logical channel number for calibration use
	 * @param reference_source_voltage	Reference voltage. This is not required if internal reference is used
	 * @param input_select				Physical input channel selection. It will use internal voltage reference if this value is 0
	 * @param use_positive_side			Physical input channel selection AnP or AnN
	 * @return CalibrationError 		Error code
	 */
	int		self_calibrate( int pga_gain_index, int channel_selection = 15, int input_select = 0, double reference_source_voltage = 0, bool use_positive_side = true );

	/** Blinks LEDs on GPIO pins */
	void blink_leds( void );
};

class NAFE13388 : public NAFE13388_Base
{
public:	
	/** Constructor to create a NAFE13388 instance */
	NAFE13388( SPI& spi, bool spi_addr = 0, bool highspeed_variant = false, int nINT = D2, int DRDY = D3, int SYN = D5, int nRESET = D6 );

	/** Destractor */
	virtual ~NAFE13388();
};

class NAFE13388_UIM : public NAFE13388_Base
{
public:	
	/** Constructor to create a NAFE13388 instance */
	NAFE13388_UIM( SPI& spi, bool spi_addr = 0, bool highspeed_variant = false, int nINT = D3, int DRDY = D4, int SYN = D6, int nRESET = D7 );

	/** Destractor */
	virtual ~NAFE13388_UIM();

	void blink_leds( void );
};

inline NAFE13388_Base::Register16 operator+( NAFE13388_Base::Register16 rn, int n )
{
    return static_cast<NAFE13388_Base::Register16>( static_cast<uint16_t>( rn ) + n );
}

inline NAFE13388_Base::Register16 operator+( int n, NAFE13388_Base::Register16 rn )
{
    return static_cast<NAFE13388_Base::Register16>( n + static_cast<uint16_t>( rn ) );
}

inline NAFE13388_Base::Register24 operator+( NAFE13388_Base::Register24 rn, int n )
{
    return static_cast<NAFE13388_Base::Register24>( static_cast<uint16_t>( rn ) + n );
}

inline NAFE13388_Base::Register24 operator+( int n, NAFE13388_Base::Register24 rn )
{
    return static_cast<NAFE13388_Base::Register24>( n + static_cast<uint16_t>( rn ) );
}

#endif //	ARDUINO_AFE_DRIVER_H
