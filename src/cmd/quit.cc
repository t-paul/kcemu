#include "kc/kc.h"
#include "kc/z80.h"
#include "ui/status.h"
#include "cmd/cmd.h"

class CMD_quit : public CMD
{
public:
  
  CMD_quit(void) : CMD("Quit")
    {
      register_cmd("emu-quit");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      Status::instance()->setMessage("*** QUIT ***");
      CMD_Repository::instance()->dump();
      z80->quit();
    }
};

__force_link(CMD_quit);
