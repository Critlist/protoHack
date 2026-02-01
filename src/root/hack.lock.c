#include "../compat.h"
#include "hack.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>

/**
 * MODERN ADDITION (2026): flock()-based locking for game and record files
 *
 * WHY: The original link()-based lock files are fragile on modern systems and
 *      can leave stale locks after crashes.
 *
 * HOW: Use flock() on dedicated lock files for game and record updates.
 *
 * PRESERVES: Single-instance game semantics and record file integrity.
 * ADDS: Robust locking and automatic release on process exit.
 */

static int game_lock_fd = -1;
static int record_lock_fd = -1;

#define GAME_LOCK_FILE "game.lock"
#define RECORD_LOCK_FILE "record.lock"
#define LOCK_TIMEOUT 10

int modern_lock_game(void)
{
	int fd;
	int attempts=0;

	fd=open(GAME_LOCK_FILE,O_CREAT|O_RDWR,0644);
	if(fd<0) {
		printf("Cannot create game lock file %s: %s\n",GAME_LOCK_FILE,strerror(errno));
		return 0;
	}
	while(attempts<LOCK_TIMEOUT) {
		if(flock(fd,LOCK_EX|LOCK_NB)==0) {
			game_lock_fd=fd;
			return 1;
		}
		if(errno!=EWOULDBLOCK && errno!=EAGAIN) {
			printf("Cannot lock game: %s\n",strerror(errno));
			close(fd);
			return 0;
		}
		if(attempts==0) printf("Another game is in progress...\n");
		sleep(1);
		attempts++;
	}
	printf("Cannot start game - lock timeout.\n");
	printf("If no other game is running, try: rm %s\n",GAME_LOCK_FILE);
	close(fd);
	return 0;
}

void modern_unlock_game(void)
{
	if(game_lock_fd!=-1) {
		if(fcntl(game_lock_fd,F_GETFD)!=-1) {
			flock(game_lock_fd,LOCK_UN);
			close(game_lock_fd);
		}
		game_lock_fd=-1;
	}
}

int modern_lock_record(void)
{
	int fd;
	int attempts=0;

	fd=open(RECORD_LOCK_FILE,O_CREAT|O_RDWR,0644);
	if(fd<0) return 0;
	while(attempts<5) {
		if(flock(fd,LOCK_EX|LOCK_NB)==0) {
			record_lock_fd=fd;
			return 1;
		}
		if(errno!=EWOULDBLOCK && errno!=EAGAIN) {
			close(fd);
			return 0;
		}
		usleep(100000);
		attempts++;
	}
	close(fd);
	return 0;
}

void modern_unlock_record(void)
{
	if(record_lock_fd!=-1) {
		if(fcntl(record_lock_fd,F_GETFD)!=-1) {
			flock(record_lock_fd,LOCK_UN);
			close(record_lock_fd);
		}
		record_lock_fd=-1;
	}
}

void modern_cleanup_locks(void)
{
	int fd;

	fd=open(GAME_LOCK_FILE,O_RDWR);
	if(fd!=-1) {
		if(flock(fd,LOCK_EX|LOCK_NB)==0) flock(fd,LOCK_UN);
		close(fd);
	}
	fd=open(RECORD_LOCK_FILE,O_RDWR);
	if(fd!=-1) {
		if(flock(fd,LOCK_EX|LOCK_NB)==0) flock(fd,LOCK_UN);
		close(fd);
	}
}
