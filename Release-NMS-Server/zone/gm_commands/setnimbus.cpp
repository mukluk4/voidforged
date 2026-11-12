#include "../client.h"
#include "../command.h"
#include "../../common/strings.h"

// #setnimbus [on|off]: show or hide your own full armor set nimbus (armor_set_nimbus table). Saved per character.
void command_setnimbus(Client *c, const Seperator *sep)
{
	const std::string arg = Strings::ToLower(sep->arg[1]);

	if (arg == "on" || arg == "off") {
		c->SetSetNimbusHidden(arg == "off");
		c->Message(
			Chat::White,
			arg == "off" ? "Your armor set nimbus is now hidden." : "Your armor set nimbus is now shown while a tier set has its full bonus."
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Your armor set nimbus is {}. Usage: #setnimbus [on|off]",
			c->IsSetNimbusHidden() ? "hidden" : "shown"
		).c_str()
	);
}
