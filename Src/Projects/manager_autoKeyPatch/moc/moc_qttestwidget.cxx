/****************************************************************************
** Meta object code from reading C++ file 'qttestwidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qttestwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qttestwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtTestWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,
      41,   13,   13,   13, 0x0a,
      77,   13,   13,   13, 0x0a,
     118,  114,   13,   13, 0x0a,
     142,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtTestWidget[] = {
    "QtTestWidget\0\0on_createCubeBtn_clicked()\0"
    "on_rotateCubeDial_valueChanged(int)\0"
    "on_scaleCubeSlider_valueChanged(int)\0"
    "pos\0ShowContextMenu(QPoint)\0"
    "OnAutoInterpTrigger()\0"
};

void QtTestWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtTestWidget *_t = static_cast<QtTestWidget *>(_o);
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

const QMetaObjectExtraData QtTestWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtTestWidget::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_QtTestWidget,
      qt_meta_data_QtTestWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtTestWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtTestWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtTestWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtTestWidget))
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
    }
    return _id;
}
QT_END_MOC_NAMESPACE
