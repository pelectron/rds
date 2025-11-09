#include "rds/qt/node.hpp"

namespace rds {

Node::~Node() {}

void Node::setFormat(Fmt fmt) {
  if (fmt == format_)
    return;
  format_ = fmt;
  emit formatChanged(format_);
}

Fmt Node::format() const { return format_; }

} // namespace rds
