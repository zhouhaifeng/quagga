/*
 * Zebra configuration command interface routine
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _ZEBRA_COMMAND_EXECUTE_H
#define _ZEBRA_COMMAND_EXECUTE_H

#include "command_local.h"
#include "command_parse.h"
#include "vty_common.h"
#include "qstring.h"
#include "mqueue.h"
#include "qpnexus.h"
#include "thread.h"

/*==============================================================================
 * This is stuff which is used to parse and then execute commands.
 */

/* State of the execution loop
 */
enum cmd_exec_state
{
  exec_null   = 0,

  exec_special,         /* not a simple command         */

  exec_fetch,
  exec_parse,
  exec_open_pipes,
  exec_execute,
  exec_success,
  exec_complete,
} ;
typedef enum cmd_exec_state cmd_exec_state_t ;

typedef struct cmd_exec* cmd_exec ;

struct cmd_exec
{
  vty           vty ;                   /* parent                       */

  qstring       line ;                  /* pointer to qstring in vf     */
  cmd_do_t      to_do ;                 /* for cli driven stuff         */

  cmd_parse_type_t  parse_type ;        /* how should parse             */
  bool          out_enabled ;           /* as required                  */
  bool          reflect_enabled ;       /* as required                  */

  cmd_parsed_t      parsed ;            /* embedded                     */

  cmd_exec_state_t  state ;             /* for cq_process               */
  qpn_nexus         locus ;             /* for cq_process               */

  cmd_return_code_t ret ;               /* for cq_process               */

  union
  {
    mqueue_block    mqb ;               /* for cq_process               */
    struct thread*  thread ;
  } cq ;
} ;

/*==============================================================================
 * Functions
 *
 */

extern cmd_exec cmd_exec_new(vty vty) ;
extern cmd_exec cmd_exec_free(cmd_exec exec) ;

extern cmd_return_code_t cmd_read_config(vty vty, cmd_command first_cmd,
                                                          bool ignore_warning) ;
extern cmd_return_code_t cmd_end(vty vty) ;
extern cmd_return_code_t cmd_exit(vty vty) ;

extern cmd_return_code_t cmd_open_pipes(vty vty) ;

extern cmd_return_code_t cmd_execute(vty vty) ;






#if 0

extern enum cmd_return_code cmd_execute_command (vty vty,
                              cmd_parse_type_t type, struct cmd_command **cmd) ;
extern enum cmd_return_code cmd_execute_command_strict (vty vty,
                          enum cmd_parse_type type, struct cmd_command **cmd) ;


extern void config_replace_string (cmd_command, char *, ...);

#endif

/*==============================================================================
 * Inlines
 */

Inline bool
cmd_is_direct(cmd_parsed parsed)
{
  return ((parsed->cmd->attr & CMD_ATTR_DIRECT) != 0) ;
} ;

#endif /* _ZEBRA_COMMAND_EXECUTE_H */