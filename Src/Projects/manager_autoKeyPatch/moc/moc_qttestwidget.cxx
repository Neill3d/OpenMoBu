/****************************************************************************
** Meta object code from reading C++ file 'qttestwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qttestwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qttestwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QtTestWidget_t {
    QByteArrayData data[8];
    char stringdata0[142];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtTestWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtTestWidget_t qt_meta_stringdata_QtTestWidget = {
    {
QT_MOC_LITERAL(0, 0, 12), // "QtTestWidget"
QT_MOC_LITERAL(1, 13, 24), // "on_createCubeBtn_clicked"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 30), // "on_rotateCubeDial_valueChanged"
QT_MOC_LITERAL(4, 70, 31), // "on_scaleCubeSlider_valueChanged"
QT_MOC_LITERAL(5, 102, 15), // "ShowContextMenu"
QT_MOC_LITERAL(6, 118, 3), // "pos"
QT_MOC_LITERAL(7, 122, 19) // "OnAutoInterpTrigger"

    },
    "QtTestWidget\0on_createCubeBtn_clicked\0"
    "\0on_rotateCubeDial_valueChanged\0"
    "on_scaleCubeSlider_valueChanged\0"
    "ShowContextMenu\0pos\0OnAutoInterpTrigger"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtTestWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x0a /* Public */,
       3,    1,   40,    2, 0x0a /* Public */,
       4,    1,   43,    2, 0x0a /* Public */,
       5,    1,   46,    2, 0x0a /* Public */,
       7,    0,   49,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::QPoint,    6,
    QMetaType::Void,

       0        // eod
};

void QtTestWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QtTestWidget *_t = static_cast<QtTestWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_createCubeBtn_clicked(); break;
        case 1: _t->on_rotateCubeDial_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->on_scaleCubeSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->ShowContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 4: _t->OnAutoInterpTrigger(); break;
        default: ;
        }
    }
}

const QMetaObject QtTestWidget::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_QtTestWidget.data,
      qt_meta_data_QtTestWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QtTestWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtTestWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QtTestWidget.stringdata0))
        return static_cast<void*>(const_cast< QtTestWidget*>(this));
    return QFrame::qt_metacast(_clname);
}

int QtTestWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
