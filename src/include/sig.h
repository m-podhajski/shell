#ifndef _SIG_H_
#define _SIG_H_

extern sigset_t wait_;
extern sigset_t sigset;
extern struct sigaction sig1;
extern struct sigaction sig2;

void add_foreground_process(int in);

void sigchld_block_();

void sigchld_unblock();

void sigchld_handler();

void set_handlers();

void prompt();

int background();

int bckgrnd_proc();

void set_background(int in);

void set_size(int in);

void set_counter(int in);

int foreground_sze();

#endif /* !_SIG_H_ */