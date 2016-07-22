/*
 * Because the stdio.h's include didn't seem to define it,
 * I just decided to do it manually, since I didn't
 * find any problem to do that.
 */

#ifndef DPRINTF_H_
int dprintf(int, const char *, ...);
#endif /* !DPRINTF_H_ */
