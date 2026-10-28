// #include "mainwindow.h"
#include "slamlauncher.h"
#include <boost/asio/io_context.hpp>

#include <QApplication>

int main(int argc, char *argv[])
{
    // QApplication a(argc, argv);
    // MainWindow w;
    // w.show();
    // return a.exec();

    boost::asio::io_context ctx;

    SlamLauncher slamLauncher(ctx);

    ctx.run();
    return 0;
}
