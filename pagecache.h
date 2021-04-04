struct pagecache{
		int dev;
		int inode_number;
		int refCount;
	  int offset;	
		char* page;
}; 
