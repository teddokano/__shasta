#include	<variant>
#include	"afe/SPI_for_AFE.h"

class SHASTA_basic : public SPI_for_AFE
{
public:
	/** Constructor to create a AFE_base instance */
	SHASTA_basic( SPI& spi, bool spi_addr );

	/** Destractor */
	virtual ~SHASTA_basic();
	

	enum class Register16 : uint16_t {
		CRC_CONF_REGS		= 0x20,
		CRC_COEF_REGS		= 0x21,
		CRC_TRIM_REGS		= 0x22,
		CRC_TRIM_REF		= 0x3F, 
		GPI_DATA			= 0x23,
		GPO_ENABLE			= 0x24,
		GPIO_FUNCTION		= 0x25,
		GPI_ENABLE			= 0x26,
		GPI_EDGE_POS		= 0x27,
		GPI_EDGE_NEG		= 0x28,
		GPO_DATA			= 0x29,
		SYS_CONFIG			= 0x2A,
		SYS_STATUS			= 0x2B,
		CK_SRC_SEL_CONFIG	= 0x30,
		GLOBAL_ALARM_ENABLE	= 0x2C,
		GLOBAL_ALARM_INT	= 0x2D,
		DIE_TEMP			= 0x2E,
		TEMP_THRS			= 0x2F,
		PN2					= 0x40,
		PN1					= 0x41,
		PN0_REV				= 0x42,
		
		AI_CONFIG0			= 0x1000 | 0x20,
		AI_CONFIG1			= 0x1000 | 0x21,
		AI_CONFIG2			= 0x1000 | 0x22,
		AI_MULTI_CH_EN		= 0x1000 | 0x23,
		AI_SYSCFG			= 0x1000 | 0x24,
		AI_STATUS			= 0x1000 | 0x25,
		AI_STATUS_OVR		= 0x1000 | 0x26,
		AI_STATUS_UDR		= 0x1000 | 0x27,
		
		AIO_CONFIG			= 0x1C00 | 0x20,
		AO_CAL_COEF			= 0x1C00 | 0x21,
		AIO_PROT_CFG		= 0x1C00 | 0x22,
		AO_SLR_CTRL			= 0x1C00 | 0x23,
		AWG_PER				= 0x1C00 | 0x24,
		AO_SYSCFG			= 0x1C00 | 0x25,
		AIO_STATUS			= 0x1C00 | 0x26
	};

	enum class Register24 : uint16_t {
		GAINCOEF0			= 0x50,
		GAINCOEF1,
		GAINCOEF2,
		GAINCOEF3,
		GAINCOEF4,
		GAINCOEF5,
		GAINCOEF6,
		GAINCOEF7,
		OFFSET_COEF0		= 0x58,
		OFFSET_COEF1,
		OFFSET_COEF2,
		OFFSET_COEF3,
		OFFSET_COEF4,
		OFFSET_COEF5,
		OFFSET_COEF6,
		OFFSET_COEF7,
		EXTRA_CAL_COEF0		= 0x60,
		EXTRA_CAL_COEF1,
		EXTRA_CAL_COEF2,
		EXTRA_CAL_COEF3,
		EXTRA_CAL_COEF4,
		EXTRA_CAL_COEF5,
		EXTRA_CAL_COEF6,
		EXTRA_CAL_COEF7,
		SERIAL1				= 0x43,
		SERIAL0,
		
		AI_DATA0			= 0x1000 | 0x30,
		AI_DATA1,
		AI_DATA2,
		AI_DATA3,
		AI_DATA4,
		AI_DATA5,
		AI_DATA6,
		AI_DATA7,
		AI_CH_OVR_THR_0		= 0x1000 | 0x38,
		AI_CH_OVR_THR_1,
		AI_CH_OVR_THR_2,
		AI_CH_OVR_THR_3,
		AI_CH_OVR_THR_4,
		AI_CH_OVR_THR_5,
		AI_CH_OVR_THR_6,
		AI_CH_OVR_THR_7,
		AI_CH_UDR_THR_0		= 0x1000 | 0x40,
		AI_CH_UDR_THR_1,
		AI_CH_UDR_THR_2,
		AI_CH_UDR_THR_3,
		AI_CH_UDR_THR_4,
		AI_CH_UDR_THR_5,
		AI_CH_UDR_THR_6,
		AI_CH_UDR_THR_7,
		
		AO_DATA				= 0x1C00 | 0x28,
		AO_OC_POS_LIMIT,
		AO_OC_NEG_LIMIT,
		AWG_AMP_MAX,
		AWG_AMP_MIN
	};

	enum Command : uint16_t {
		CMD_CLEAR_ALARM		= 0x12,
		CMD_RESET			= 0x14,
		CMD_CLEAR_REG		= 0x15,
		CMD_RELOAD			= 0x16,
		CMD_CALC_CRC_CONFIG	= 0x2006,
		CMD_CALC_CRC_COEF	= 0x2007,
		CMD_CALC_CRC_FAC	= 0x2008,
		
		CMD_CH0				= 0x1000,
		CMD_CH1				= 0x1001,
		CMD_CH2				= 0x1002,
		CMD_CH3				= 0x1003,
		CMD_CH4				= 0x1004,
		CMD_CH5				= 0x1005,
		CMD_CH6				= 0x1006,
		CMD_CH7				= 0x1007,
		CMD_ADC_ABORT		= 0x1010,
		CMD_END				= 0x1012,
		CMD_CLEAR_DATA		= 0x1013,
		CMD_SS				= 0x3000,
		CMD_SC				= 0x3001,
		CMD_MM				= 0x3002,
		CMD_MC				= 0x3003,
		CMD_MS				= 0x3004,
		CMD_BURST_DATA		= 0x3005,
		
		CMD_WGEN_STOP		= 0x1C00,
		CMD_WGEN_START		= 0x1C01,
		CMD_CLEAR_DAC0		= 0x1C02,
		CMD_AO_ABORT		= 0x1C03,
		CMD_AO_ABORT_HIZ	= 0x1C04,
		CMD_CISW_ABORT		= 0x1C05,
		CMD_CISW_ABORT_HIZ	= 0x1C06,
	};

	using	RegisterVariant	= std::variant<Register16, Register24>;
	using	RegVct			= std::vector<RegisterVariant>;
	
	/** Command
	 *	
	 * @param com "Comand" type or uint16_t value
	 */
	virtual void		command( uint16_t com );

	/** Write register
	 *
	 *	Writes register. Register width is selected by reg type (Register16 ot Register24)
	 * @param reg register specified by Register16 member
	 */
	virtual void		reg( Register16 r, uint16_t value );

	/** Write register
	 *
	 *	Writes register. Register width is selected by reg type (Register16 ot Register24)
	 * @param reg register specified by Register24 member
	 */
	virtual void		reg( Register24 r, uint32_t value );

	/** Read register
	 *
	 *	Reads register. Register width is selected by reg type (Register16 ot Register24)
	 * @param reg register specified by Register16 member
	 * @return readout value
	 */
	virtual uint16_t	reg( Register16 r );

	/** Read register
	 *
	 *	Reads register. Register width is selected by reg type (Register16 ot Register24)
	 * @param reg register specified by Register24 member
	 * @return readout value
	 */
	virtual uint32_t	reg( Register24 r );
	
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
	

};
