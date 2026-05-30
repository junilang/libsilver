int ZZentry(SilverTestContext *ctx) {
	pid_t pid = fork();
	if (pid == 0) {
		auto error = execve("/bin/sleep", (char*[]){"sleep", "2", nullptr}, (char*[]){nullptr});
		FPRINT("ERRROR: ",error,"\n");
	} else if (pid > 0) {
		int status;
		pid_t result = waitpid(pid, &status, 0);
		FPRINT(stdout, "done ",result," ",status,"\n");
	}

	return 0;
}
