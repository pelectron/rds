#include "rds/qt/widget.hpp"

#include <QApplication>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
  using namespace std::string_view_literals;
  try {
  } catch (const std::exception &e) {
  }
  QApplication a(argc, argv);
  qDebug() << "Application: " << a.applicationName();
  rds::qt::Widget w(rds::qt::Options::All);
  w.load_file("../../../max17320.toml");
  w.show();
  return a.exec();
}
