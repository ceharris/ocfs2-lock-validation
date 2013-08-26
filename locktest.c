#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>

#define DEFAULT_DELAY     10     /* sleep time if not specified */
#define FILE_PERMS        0644   /* perms assigned if file created */


#define LOCKFN_FLOCK "flock"
#define LOCKFN_LOCKF "lockf"
#define LOCKFN_FCNTL "fcntl"

/* exit codes */
enum {
  EXIT_OK,        /* all went well */
  EXIT_SYNTAX,    /* command syntax error */
  EXIT_OPEN,      /* error opening the file */
  EXIT_LOCK       /* error acquiring or releasing the lock */
};

/* a pointer to a lock function */
int (*lockfn_t)(int fd);

/* a lock/unlock function pair */
struct lockfn_set_t {
  int (*lock)(int fd);
  int (*unlock)(int fd);
};

/* program configuration */
struct config_t {
  const char *name;
  const char *file;
  unsigned int delay;
  struct lockfn_set_t *fn;
};

/* prototypes */
struct config_t *configuration();
void show_usage(const char *prog);
int open_file(struct config_t *config);
int flock_lock(int fd);
int flock_unlock(int fd);
int lockf_lock(int fd);
int lockf_unlock(int fd);
int fcntl_lock(int fd);
int fcntl_unlock(int fd);
void init_lock(struct flock *fl);

/* function pair for 'flock' locking */
static struct lockfn_set_t flock_set = { flock_lock, flock_unlock };
/* function pair for 'lockf' locking */
static struct lockfn_set_t lockf_set = { lockf_lock, lockf_unlock };
/* function pair for 'fcntl' locking */
static struct lockfn_set_t fcntl_set = { fcntl_lock, fcntl_unlock };

/**
 * Main entry point
 */
int 
main(int argc, char *argv[]) 
{
  int fd;
  struct config_t *config;

  config = configuration(argc, argv);
  if (config == NULL) {
    return EXIT_SYNTAX;
  }

  printf("opening file %s\n", config->file);
  fd = open_file(config);
  if (fd < 0) {
    fputs("failed to open file\n", stderr);
    return EXIT_OPEN;
  }

  printf("using %s to acquire lock\n", config->name);
  if (config->fn->lock(fd) < 0) {
    fputs("failed to acquire lock\n", stderr);
    return EXIT_LOCK;
  } 

  puts("lock acquired");
  printf("sleeping for %d seconds\n", config->delay);
  sleep(config->delay);

  if (config->fn->unlock(fd) < 0) {
    fputs("failed to release lock\n", stderr);
    return EXIT_LOCK;
  }

  puts("lock released");
  close(fd);

  return EXIT_OK;
}

/**
 * Fills in a program configuration structure from command line args.
 * return: pointer to a static struct
 */
struct config_t *
configuration(int argc, char *argv[])
{
  const char *prog;
  int opt;
  static struct config_t config;

  prog = argv[0];
  argc--;
  argv++;

  if (argc < 2 || argc > 3) {
    show_usage(prog);
    return NULL;
  }

  if (strcmp(LOCKFN_FLOCK, argv[0]) == 0) {
    config.fn = &flock_set;
  }
  else if (strcmp(LOCKFN_LOCKF, argv[0]) == 0) {
    config.fn = &lockf_set;
  }
  else if (strcmp(LOCKFN_FCNTL, argv[0]) == 0) {
    config.fn = &fcntl_set;
  }
  else {
    show_usage(prog);
    return NULL;
  }

  config.name = argv[0];
  config.file = argv[1];

  if (argc == 3) {
    config.delay = strtol(argv[2], NULL, 10);
    if (config.delay == 0) {
      show_usage(prog);
      return NULL;
    }
  }
  else {
    config.delay = DEFAULT_DELAY;
  }

  return &config;

}

/**
 * Show program usage instructions.
 */
void
show_usage(const char *prog)
{
  fprintf(stderr, "usage: %s lock-fn filename [duration]\n", prog);
  fprintf(stderr, "where 'lock-fn' is one of %s, %s, or %s\n",
          LOCKFN_FLOCK, LOCKFN_LOCKF, LOCKFN_FCNTL);
  fprintf(stderr, "and 'duration' is the delay (seconds) after "
                  "acquiring the lock (default %d)\n", DEFAULT_DELAY);
}

/**
 * Opens the file specified by the configuration, creating if necessary.
 * return: file descriptor or -1 if an error occurred
 */
int
open_file(struct config_t *config)
{
  int fd;

  fd = open(config->file, O_WRONLY | O_CREAT, FILE_PERMS);
  if (fd < 0) {
    perror("open");
    return -1;
  }
 
  return fd;
}

/**
 * Locks a file using flock(2)
 * param:   fd  descriptor of the file to lock
 * return:  return code from flock
 */
int
flock_lock(int fd)
{
  int rc;
  
  rc = flock(fd, LOCK_EX | LOCK_NB);
  if (rc < 0 || errno == EWOULDBLOCK) {
    puts("waiting for lock");
    rc = flock(fd, LOCK_EX);
  }
  if (rc < 0) {
    perror("flock");
  }
  return rc;
}

/**
 * Unlocks a file using flock(2)
 * param:   fd  descriptor of the file to lock
 * return:  return code from flock
 */
int
flock_unlock(int fd)
{
  int rc;

  rc = flock(fd, LOCK_UN);
  if (rc < 0) {
    perror("flock");
  }

  return rc;
}

/**
 * Locks a file using lockf(3)
 * param:   fd  descriptor of the file to lock
 * return:  return code from lockf
 */
int 
lockf_lock(int fd)
{
  int rc;

  if (lseek(fd, 0, SEEK_SET) < 0) {
    perror("lseek");
    return -1;
  }

  rc = lockf(fd, F_TLOCK, 0);
  if (rc < 0 && (errno == EACCES || errno == EAGAIN)) {
    rc = lockf(fd, F_LOCK, 0);
  }
  if (rc < 0) {
    perror("lockf");
  }
  return rc;
}

/**
 * Unlocks a file using lockf(3)
 * param:   fd  descriptor of the file to lock
 * return:  return code from lockf
 */
int 
lockf_unlock(int fd)
{
  int rc;

  if (lseek(fd, 0, SEEK_SET) < 0) {
    perror("lseek");
    return -1;
  }

  rc = lockf(fd, F_ULOCK, 0);
  if (rc < 0) {
    perror("lockf");
  }
  return rc;
}

/**
 * Locks a file using fcntl(2)
 * param:   fd  descriptor of the file to lock
 * return:  return code from fcntl
 */
int
fcntl_lock(int fd)
{
  int rc;
  struct flock fl;

  init_lock(&fl);
  rc = fcntl(fd, F_SETLK, &fl);

  if (rc < 0 && (errno == EACCES || errno == EAGAIN)) {
    puts("waiting for lock");
    rc = fcntl(fd, F_SETLKW, &fl);
  }
  if (rc < 0) {
    perror("fcntl");
  }
  return rc;
}

/**
 * Unlocks a file using fcntl(2)
 * param:   fd  descriptor of the file to lock
 * return:  return code from fcntl
 */
int
fcntl_unlock(int fd)
{
  int rc;
  struct flock fl;

  init_lock(&fl);
  rc = fcntl(fd, F_UNLCK, &fl);
  if (rc < 0) {
    perror("fcntl");
  }

  return rc;
}

/**
 * Initializes a lock structure used by fcntl
 * param:   fl  pointer to subject lock structure
 */
void
init_lock(struct flock *fl)
{
  fl->l_type = F_WRLCK;
  fl->l_whence = SEEK_SET;
  fl->l_start = 0;
  fl->l_len = 0;
  fl->l_pid = getpid();
}
