/*
 * HackDS Init System
 * Minimal init process for HackDS gaming OS
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#define VERSION "0.1.0"

static void mount_filesystems(void);
static void setup_environment(void);
static void spawn_menu(void);
static void reap_zombies(int sig);
static void handle_shutdown(int sig);

static volatile int shutting_down = 0;
static pid_t menu_pid = -1;

int main(int argc, char *argv[]) {
    printf("HackDS Init v%s starting...\n", VERSION);

    // We must be PID 1
    if (getpid() != 1) {
        fprintf(stderr, "Error: init must be run as PID 1\n");
        return 1;
    }

    // Set up signal handlers
    signal(SIGCHLD, reap_zombies);
    signal(SIGTERM, handle_shutdown);
    signal(SIGINT, handle_shutdown);

    // Mount essential filesystems
    mount_filesystems();

    // Set up environment
    setup_environment();

    // Create device nodes if they don't exist
    mknod("/dev/null", S_IFCHR | 0666, makedev(1, 3));
    mknod("/dev/zero", S_IFCHR | 0666, makedev(1, 5));
    mknod("/dev/tty", S_IFCHR | 0666, makedev(5, 0));

    // Set hostname
    sethostname("hackds", 6);

    printf("HackDS Init: System initialized\n");

    // Launch the game menu
    spawn_menu();

    // Main loop - wait for signals and respawn menu if needed
    while (!shutting_down) {
        sleep(1);

        // Check if menu crashed
        if (menu_pid > 0) {
            int status;
            pid_t result = waitpid(menu_pid, &status, WNOHANG);
            if (result > 0) {
                printf("Menu process exited, respawning...\n");
                sleep(1);
                spawn_menu();
            }
        }
    }

    // Shutdown sequence
    printf("HackDS Init: Shutting down...\n");

    // Kill menu process
    if (menu_pid > 0) {
        kill(menu_pid, SIGTERM);
        waitpid(menu_pid, NULL, 0);
    }

    // Unmount filesystems
    umount("/proc");
    umount("/sys");
    umount("/dev");

    sync();

    // Reboot or halt
    reboot(RB_AUTOBOOT);

    return 0;
}

static void mount_filesystems(void) {
    // Create mount points
    mkdir("/proc", 0755);
    mkdir("/sys", 0755);
    mkdir("/dev", 0755);
    mkdir("/tmp", 1777);
    mkdir("/run", 0755);

    // Mount proc
    if (mount("proc", "/proc", "proc", 0, NULL) != 0) {
        fprintf(stderr, "Failed to mount /proc: %s\n", strerror(errno));
    }

    // Mount sysfs
    if (mount("sysfs", "/sys", "sysfs", 0, NULL) != 0) {
        fprintf(stderr, "Failed to mount /sys: %s\n", strerror(errno));
    }

    // Mount devtmpfs
    if (mount("devtmpfs", "/dev", "devtmpfs", 0, "mode=0755") != 0) {
        fprintf(stderr, "Failed to mount /dev: %s\n", strerror(errno));
    }

    // Mount tmpfs for /tmp
    if (mount("tmpfs", "/tmp", "tmpfs", 0, "mode=1777") != 0) {
        fprintf(stderr, "Failed to mount /tmp: %s\n", strerror(errno));
    }

    // Mount tmpfs for /run
    if (mount("tmpfs", "/run", "tmpfs", 0, "mode=0755") != 0) {
        fprintf(stderr, "Failed to mount /run: %s\n", strerror(errno));
    }
}

static void setup_environment(void) {
    // Set essential environment variables
    setenv("PATH", "/system/bin:/usr/bin:/bin", 1);
    setenv("HOME", "/", 1);
    setenv("TERM", "linux", 1);
    setenv("HACKDS_VERSION", VERSION, 1);

    // Create necessary directories
    mkdir("/games", 0755);
    mkdir("/mods", 0755);
    mkdir("/settings", 0755);
    mkdir("/hacks", 0755);

    // Set permissions
    chmod("/tmp", 01777);
}

static void spawn_menu(void) {
    menu_pid = fork();

    if (menu_pid < 0) {
        fprintf(stderr, "Failed to fork menu process: %s\n", strerror(errno));
        return;
    }

    if (menu_pid == 0) {
        // Child process - execute menu
        char *args[] = {"/system/bin/hackds-menu", NULL};
        char *env[] = {
            "PATH=/system/bin:/usr/bin:/bin",
            "HOME=/",
            "TERM=linux",
            "DISPLAY=:0",
            NULL
        };

        execve("/system/bin/hackds-menu", args, env);

        // If we get here, exec failed
        fprintf(stderr, "Failed to execute menu: %s\n", strerror(errno));
        exit(1);
    }

    printf("Menu spawned with PID %d\n", menu_pid);
}

static void reap_zombies(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

static void handle_shutdown(int sig) {
    (void)sig;
    shutting_down = 1;
}
