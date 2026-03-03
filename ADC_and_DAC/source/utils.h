/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"
#include	"afe/NAFE33352.h"

using enum	NAFE33352_Base::Register16;
using enum	NAFE33352_Base::Register24;
using enum	NAFE33352_Base::Command;

using raw_t			= NAFE33352_Base::raw_t;
using RegVct		= NAFE33352_Base::RegVct;
using ch_setting_t	= NAFE33352_Base::ch_setting_t;

void	reg_dump( RegVct reg_vctr );
void	reg_dump( NAFE33352_Base::Register24 addr, int length );
void	logical_ch_config_view( void );
void	table_view( int size, int cols, std::function<void(int)> view, std::function<void(void)> linefeed = nullptr );
