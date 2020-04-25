#include <QCoreApplication>

#include <QIODevice>
#include <qfcgiapp.h>

#include <signal.h>

static void quit_handler(int signal) {
    Q_UNUSED(signal)
    qApp->quit();
}

int main(int argc, char *argv[])
{
    QFcgiApp a(argc, argv);

    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = &quit_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGHUP, &sa, nullptr) != 0 || sigaction(SIGTERM, &sa, nullptr) != 0 || sigaction(SIGINT, &sa, nullptr) != 0)
    {
        perror(argv[0]);
        return 1;
    }

    return a.exec();
}
