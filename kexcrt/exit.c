void __exit(void);

void exit(int v)
{
	__exit();
	ExitProcess(v);
}
