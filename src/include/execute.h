#ifndef _EXECUTE_H_
#define _EXECUTE_H_

void redirect(command *com);

void check_line(char *bufor, int size);

void exec_command(command *com, int *pipe1, int *pipe2);

void exec_pipeline(pipeline pipeline_);

void execute(char *bufor, int size);

#endif /* !_EXECUTE_H_ */
