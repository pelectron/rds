#ifndef RDS_QT_COLUMN_HPP
#define RDS_QT_COLUMN_HPP

#include "rds/qt/common.hpp"
#include "rds/qt/register.hpp"

#include <QVariant>
#include <functional>

namespace rds {

/**
 * The name/id of the data to diplay in a column.
 */
enum class DataId : uint32_t {
  Name = 0,
  AddrPos,
  Value,
  Initial,
  Unit,
  Description,
  ZeroCodeValue,
  Step,
  ExtraData,
  Custom
};

Q_ENUM_NS(DataId);

/**
 * This class describes a column, i.e. it contains the necessary information to
 * label a column in a tree view and knows how to extract the relevant data.
 * @addtogroup qrds
 */
class QColumn {
public:
  /**
   * The FieldAccess is used to retrieve the data from a field for a column.
   * It takes a QField and a Qt::ItemDataRole as parameters and returns a
   * QVariant containing the relevant data (or an empty variant, if the field
   * does not have any column relevant data). You can ignore the role and
   * provide a value regardlessof it.
   */
  using FieldAccess = std::function<QVariant(const QField &f, int role)>;

  /**
   * The RegisterAccess is used to retrieve the data from a register for a
   * column. It takes a QRegister and a Qt::ItemDataRole as parameters and
   * returns a QVariant containing the relevant data (or an empty variant, if
   * the register does not have any column relevant data). You can ignore the
   role and provide a value regardlessof it.

   */
  using RegisterAccess = std::function<QVariant(const QRegister &r, int role)>;

  QColumn() = default;
  QColumn(const QColumn &other) = default;
  QColumn(QColumn &&other) = default;
  QColumn &operator=(const QColumn &other) = default;
  QColumn &operator=(QColumn &&other) = default;

  /**
   * create a default column for the given index. idx must be smaller or equal
   * to DataIndex::STEP.
   * @param idx the data index
   */
  QColumn(DataId id);

  /**
   * create a column for extra data.
   * @param label the text to display for the column label
   * @param name the name of the data
   */
  QColumn(std::string_view label, std::string_view name);

  /**
   * creates a custom column. A function to access the corresponding field and
   * register data must be supplied.
   *
   * @param label the text to display for the column label
   * @param get_field_data the field accessor
   * @param get_register_data the register accessor
   */
  QColumn(std::string_view label, FieldAccess get_field_data,
          RegisterAccess get_register_data);

private:
  friend class QModel;
  friend class QDevice;
  friend class QGroup;
  friend class QRegister;
  friend class QField;

  DataId id;
  QString header_label;
  QString data_name;
  FieldAccess field_data;
  RegisterAccess register_data;
};

namespace cols {
const inline QColumn unit(
    "unit value",
    [](const QField &f, int role) -> QVariant {
      if (f.unit().isEmpty())
        return {};

      return QString::number(f.value() * f.step() + f.zeroCodeValue()) + " " +
             f.unit();
    },
    [](const QRegister &r, int role) -> QVariant {
      if (r.unit().isEmpty())
        return {};

      return QString::number(r.value() * r.step() + r.zeroCodeValue()) + " " +
             r.unit();
    });
} // namespace cols
} // namespace rds

#endif
