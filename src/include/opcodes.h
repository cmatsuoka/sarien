/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#ifndef __AGI_OPCODES_H
#define __AGI_OPCODES_H

#ifdef __cplusplus
extern "C"{
#endif

int	run_logic		(int);
void	execute_agi_command	(UINT8, UINT16, UINT8 *);

void	cmd_accept_input	(void);
void	cmd_add			(UINT8, UINT8);
void	cmd_add_to_pic		(UINT8, UINT8, UINT8, UINT8, UINT8, UINT8, UINT8);
void	cmd_animate_obj		(UINT8);
void	cmd_assign		(UINT8, UINT8);
void	cmd_block		(UINT8, UINT8, UINT8, UINT8);
void	cmd_call		(UINT8);
void	cmd_cancel_line		(void);
void	cmd_clear_lines		(UINT8, UINT8, UINT8);
void	cmd_clear_text_rect	(UINT8, UINT8, UINT8, UINT8, UINT8);
void	cmd_close_dialogue	(void);
void	cmd_close_window	(void);
void	cmd_config_screen	(UINT8, UINT8, UINT8);
void	cmd_cur_cel		(UINT8, UINT8);
void	cmd_cur_loop		(UINT8, UINT8);
void	cmd_cur_view		(UINT8, UINT8);
void	cmd_cycle_time		(UINT8, UINT8);
void	cmd_dec			(UINT8);
void	cmd_disable_item	(UINT8);
void	cmd_discard_pic		(void);
void	cmd_discard_view	(UINT8);
void	cmd_display		(UINT8, UINT8, UINT8, UINT8);
void	cmd_distance		(UINT8, UINT8, UINT8);
void	cmd_div			(UINT8, UINT8);
void	cmd_draw		(UINT8);
void	cmd_draw_pic		(UINT8);
void	cmd_drop		(UINT8);
void	cmd_echo_line		(void);
void	cmd_ego_control		(void);
void	cmd_enable_item		(UINT8);
void	cmd_end_of_loop		(UINT8, UINT8);
void	cmd_erase		(UINT8);
void	cmd_fix_loop		(UINT8);
void	cmd_follow_ego		(UINT8, UINT8, UINT8);
void	cmd_force_update	(UINT8);
void	cmd_get			(UINT8);
void	cmd_get_dir		(UINT8, UINT8);
void	cmd_get_num		(UINT8, UINT8, UINT8);
void	cmd_get_posn		(UINT8, UINT8, UINT8);
void	cmd_get_priority	(UINT8, UINT8);
void	cmd_get_roomv		(UINT8, UINT8);
void	cmd_get_string		(UINT8, UINT8, UINT8, UINT8, UINT8, UINT8);
void	cmd_gfx			(void);
void	cmd_ignore_blocks	(UINT8);
void	cmd_ignore_horizon	(UINT8);
void	cmd_ignore_objs		(UINT8);
void	cmd_inc			(UINT8);
void	cmd_init_disk		(void);
void	cmd_init_joystick	(void);
void	cmd_last_cel		(UINT8, UINT8);
void	cmd_lindirect		(UINT8, UINT8);
void	cmd_load_game		(void);
void	cmd_load_logic		(UINT8);
void	cmd_load_pic		(UINT8);
void	cmd_load_sound		(UINT8);
void	cmd_load_view		(UINT8);
void	cmd_log			(void);
void	cmd_menu_input		(void);
void	cmd_move_obj		(UINT8, UINT8, UINT8, UINT8, UINT8);
void	cmd_mul			(UINT8, UINT8);
void	cmd_new_room		(UINT8);
void	cmd_norm_motion		(UINT8);
void	cmd_normal_cycling	(UINT8);
void	cmd_obj_statusv		(void);
void	cmd_object_on_any	(UINT8);
void	cmd_object_on_land	(UINT8);
void	cmd_object_on_water	(UINT8);
void	cmd_observe_blocks	(UINT8);
void	cmd_observe_horizon	(UINT8);
void	cmd_observe_objs	(UINT8);
void	cmd_open_dialogue	(void);
void	cmd_overlay_pic		(UINT8);
void	cmd_parse		(UINT8, UINT8);
void	cmd_pause		(void);
void	cmd_play_sound		(UINT8, UINT8);
void	cmd_position		(UINT8, UINT8, UINT8);
void	cmd_print		(UINT8, UINT8);
void	cmd_print_at		(UINT8, UINT8, SINT8, SINT8, SINT8);
void	cmd_prog_control	(void);
void	cmd_put			(UINT8, UINT8);
void	cmd_quit		(UINT8);
void	cmd_rand_num		(UINT8, UINT8, UINT8);
void	cmd_release_loop	(UINT8);
void	cmd_release_priority	(UINT8);
void	cmd_repos_to		(UINT8, UINT8, UINT8);
void	cmd_reset		(UINT8);
void	cmd_reset_scan_start	(UINT8);
void	cmd_restart_game	(void);
void	cmd_reverse_cycle	(UINT8);
void	cmd_reverse_loop	(UINT8, UINT8);
void	cmd_rindirect		(UINT8, UINT8);
void	cmd_save_game		(void);
void	cmd_script_size		(UINT8);
void	cmd_set			(UINT8);
void	cmd_set_cel		(UINT8, UINT8);
void	cmd_set_cur_char	(UINT8, UINT8);
void	cmd_set_dir		(UINT8, UINT8);
void	cmd_set_game_id		(UINT8, UINT8);
void	cmd_set_horizon		(UINT8);
void	cmd_set_key		(UINT8, UINT8, UINT8);
void	cmd_set_loop		(UINT8, UINT8);
void	cmd_set_menu		(UINT8, UINT8);
void	cmd_set_menu_item	(UINT8, UINT8, UINT8);
void	cmd_set_num_loops	(UINT8, UINT8);
void	cmd_set_posn		(UINT8, UINT8, UINT8);
void	cmd_set_priority	(UINT8, UINT8);
void	cmd_set_scan_start	(UINT8, UINT16 ip);
void	cmd_set_string		(UINT8, UINT8, UINT8);
void	cmd_set_text_attr	(UINT8, UINT8);
void	cmd_set_upper_left	(void);
void	cmd_set_view		(UINT8, UINT8);
void	cmd_shake_screen	(UINT8);
void	cmd_show_mem		(void);
void	cmd_show_obj		(UINT8);
void	cmd_show_pic		(void);
void	cmd_show_pri_screen	(void);
void	cmd_start_cycling	(UINT8);
void	cmd_start_motion	(UINT8);
void	cmd_start_update	(UINT8);
void	cmd_status		(void);
void	cmd_status_line_off	(void);
void	cmd_status_line_on	(void);
void	cmd_step_size		(UINT8, UINT8);
void	cmd_step_time		(UINT8, UINT8);
void	cmd_stop_cycling	(UINT8);
void	cmd_stop_input		(void);
void	cmd_stop_motion		(UINT8);
void	cmd_stop_sound		(void);
void	cmd_stop_update		(UINT8);
void	cmd_sub			(UINT8, UINT8);
void	cmd_submit_menu		(void);
void	cmd_toggle		(UINT8);
void	cmd_toggle_monitor	(void);
void	cmd_trace_info		(void);
void	cmd_trace_on		(void);
void	cmd_txt			(void);
void	cmd_unanimate_all	(void);
void	cmd_unblock		(void);
void	cmd_unk_170		(void);
void	cmd_unk_171		(void);
void	cmd_unk_172		(void);
void	cmd_unk_173		(void);
void	cmd_unk_174		(void);
void	cmd_unk_175		(void);
void	cmd_unk_176		(void);
void	cmd_unk_177		(void);
void	cmd_unk_178		(void);
void	cmd_unk_179		(void);
void	cmd_unk_180		(void);
void	cmd_unk_181		(void);
void	cmd_version		(void);
void	cmd_wander		(UINT8);
void	cmd_word_to_string	(UINT8, UINT8);

#ifdef __cplusplus
extern "C" {
#endif

struct sarien_debug {
	int enabled;
	int opcodes;
	int logic0;
	int steps;
};

struct agi_logicnames {
#if 1 /* ifndef NO_DEBUG */
	UINT8	*name;
#endif
	UINT16	num_args;
	UINT16	arg_mask;
};

extern	struct agi_logicnames	logic_names_test[];
extern	struct agi_logicnames	logic_names_cmd[];
extern	struct agi_logicnames	logic_names_if[];

void	debug_console	(int, int, char *);
int	test_if_code	(int);
void	new_room	(int);
void	break_copy_protection (int);

#ifdef __cplusplus
};
#endif

#endif /* __AGI_OPCODES_H */
