/* parse.c */void	SendStringAsIfTyped(struct WindRec *tw, char *string, short len);void	Parseunload(void);void	parse(struct WindRec *tw, unsigned char *st, short cnt);void	SendNAWSinfo(WindRec *s, short horiz, short vert);void	telnet_send_initial_options(WindRec *tw);void	send_do(short port, short option);void	send_dont(short port, short option);void	otpauto(struct WindRec *, char *, short);