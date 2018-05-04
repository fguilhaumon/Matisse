#ifndef GRAPHICALCHARTER_H
#define GRAPHICALCHARTER_H

#include "libmatissecommon_global.h"

/* COLORS */
#define MATISSE_BLACK           "#252a31"

/* FONTS */
#define MATISSE_FONT_TYPE               "Montserrat"
#define MATISSE_FONT_DEFAULT_SIZE_PT    12

/* dpi value used as reference for scaling */
#define REF_DPI                         98

/* WIDGET SIZES AND MARGINS FOR A REFERENCE DPI (then adapted to other dpi) */
#define MAIN_WINDOW_MIN_WIDTH          1280
#define MAIN_WINDOW_MIN_HEIGHT          800
#define PARAM_LABEL_WIDTH_NOWRAP        180
#define PARAM_LABEL_WIDTH_WRAP          360
#define PARAM_SPINBOX_WIDTH             90
#define PARAM_TABLE_COL_WIDTH_MAX       65
#define PARAM_TABLE_ROW_HEIGHT          20
#define PARAM_TABLE_WIDTH_NOWRAP_MAX    175
#define PARAM_TABLE_CELL_PADDING        5
#define PARAM_WIDGET_ALIGN_HEIGHT_THRE  40
#define PARAM_WIDGET_FIELD_HSPACING     3
#define PARAM_GROUP_MARGIN_TOP          10
#define PARAM_GROUP_MARGIN_BOTTOM       10

#define ASSEMBLY_PROPS_LABEL_WIDTH      140

namespace MatisseCommon {

class LIBMATISSECOMMONSHARED_EXPORT GraphicalCharter
{
public:
    static GraphicalCharter& instance();

    GraphicalCharter(GraphicalCharter const&) = delete;        // Don't forget to disable copy
    void operator=(GraphicalCharter const&) = delete;   // Don't forget to disable copy

    double ptToPx(double _pt) {
        return _pt/72*m_dpi;
    }

    double pxToPt(double _px)
    {
        return _px*72/m_dpi;
    }

    double dpi() const;

    int dpiScaled(int _ref_pixel_size);

private:
    GraphicalCharter();       // forbid create instance outside
    ~GraphicalCharter();      // forbid to delete instance outside
    double m_dpi;
};

}

#endif // GRAPHICALCHARTER_H
