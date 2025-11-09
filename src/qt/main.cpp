#include "rds/qt/widget.hpp"

#include <QApplication>
using namespace rds;
int main(int argc, char **argv) {
  QApplication a(argc, argv);
  rds::Options opts{
      rds::without(Options::All, Options::WriteButton | Options::ReadButton)};
  Widget w(opts);
  w.load_file("../max17320.toml");
  w.show();
  return a.exec();
}
