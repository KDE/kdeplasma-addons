/*
*   Copyright 2009 Matteo Agostinelli <agostinelli@gmail.com>
*   Copyright 2003-2007  Niklas Knutsson <nq@altern.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "qalculate_labels.h"
#include "qalculate_settings.h"

#include <string>
using namespace std;

#define TEXT_TAGS   "<font size=6>"
#define TEXT_TAGS_END   "</font>"
#define TEXT_TAGS_SMALL   "<font size=5>"
#define TEXT_TAGS_SMALL_END  "</font>"
#define TEXT_TAGS_XSMALL  "<font size=4>"
#define TEXT_TAGS_XSMALL_END  "</font>"

#define STR_MARKUP_ADD_SMALL(str, str_add)    if(ips.power_depth > 0) {str += TEXT_TAGS "<sup>"; str += str_add; str += "</sup>" TEXT_TAGS_END;} else {str += TEXT_TAGS_SMALL; str += str_add; str += TEXT_TAGS_SMALL_END;}

#define STR_MARKUP_ADD(str, str_add)    if(ips.power_depth > 0) {str += TEXT_TAGS "<sup>"; str += str_add; str += "</sup>" TEXT_TAGS_END;} else {str += str_add;}

#define STR_MARKUP_PREPEND(str, str_add)    QString str_prepend; if(ips.power_depth > 0) {str_prepend += TEXT_TAGS "<sup>"; str_prepend += str_add; str_prepend += "</sup>" TEXT_TAGS_END;} else {str_prepend += str_add;} str.prepend(str_prepend);

#define STR_MARKUP_BEGIN(str)      if(ips.power_depth > 0) {str += TEXT_TAGS "<sup>";}
#define STR_MARKUP_END(str)      if(ips.power_depth > 0) {str +="</sup>" TEXT_TAGS_END;}

#define STR_MARKUP_BEGIN_SMALL(str)     if(ips.power_depth > 0) {str += TEXT_TAGS_SMALL "<sup>";} else {str += TEXT_TAGS_SMALL;}
#define STR_MARKUP_END_SMALL(str)     if(ips.power_depth > 0) {str +="</sup>" TEXT_TAGS_SMALL_END;} else {str += TEXT_TAGS_SMALL_END;}

#define STR_MARKUP_BEGIN_CURSIVE(str)     if(ips.power_depth > 0) {str += TEXT_TAGS "<i><sup>";} else {str += "<i>";}
#define STR_MARKUP_END_CURSIVE(str)     if(ips.power_depth > 0) {str +="</sup></i>" TEXT_TAGS_END;} else {str += "</i>";}

QString QalculateLabels::drawStructure(MathStructure& m, const PrintOptions& po, InternalPrintStruct ips)
{
    QString mstr;



    //string result_str = m.print(po);
    InternalPrintStruct ips_n = ips;

    switch (m.type()) {
    case STRUCT_NUMBER: {
        string exp = "";
        bool exp_minus;
        ips_n.exp = &exp;
        ips_n.exp_minus = &exp_minus;
        STR_MARKUP_BEGIN(mstr);
        mstr += m.number().print(po, ips_n).c_str();
        if (!exp.empty()) {
            if (po.lower_case_e) {
                mstr += 'e';
            } else {
                STR_MARKUP_ADD_SMALL(mstr, "E");
            }
            if (exp_minus) {
                mstr += '-';
            }
            mstr += exp.c_str();
        }
        if (po.base != BASE_DECIMAL && po.base != BASE_HEXADECIMAL && po.base > 0 && po.base <= 36) {
            if (ips.power_depth == 0) {
                mstr += "<sub>";
                mstr += QString::number(po.base);
                mstr += "</sub>";
            } else {
                mstr += TEXT_TAGS_SMALL "<sup>";
                mstr += QString::number(po.base);
                mstr += "</sup>" TEXT_TAGS_SMALL_END;
            }
        }

        if (m.number().isInteger() && ips.depth == 0 && m_qalculateSettings->showOtherBases())
        {
          mstr += TEXT_TAGS_XSMALL;
          PrintOptions po2 = po;
          if (m_qalculateSettings->showBinary() && po.base != 2)
          {
            po2.base = 2;
            mstr += "<br>0b";
            mstr += m.number().print(po2, ips_n).c_str();
          }
          if (m_qalculateSettings->showOctal() && po.base != 8)
          {
            po2.base = 8;
            mstr += "<br>0o";
            mstr += m.number().print(po2, ips_n).c_str();
          }
          if (m_qalculateSettings->showDecimal() && po.base != 10)
          {
            po2.base = 10;
            po2.min_exp = EXP_NONE;
            mstr += "<br>0d";
            mstr += m.number().print(po2, ips_n).c_str();
          }
          if (m_qalculateSettings->showHexadecimal() && po.base != 16)
          {
            po2.base = 16;
            mstr += "<br>0x";
            mstr += m.number().print(po2, ips_n).c_str();
          }
          mstr += TEXT_TAGS_XSMALL_END;
        }

        STR_MARKUP_END(mstr);
        break;
    }

    case STRUCT_SYMBOLIC: {
        result_parts.push_back(m);
        mstr = "<a name=\"";
        mstr += QString::number(result_parts.size());
        mstr += "\">";
        STR_MARKUP_BEGIN_CURSIVE(mstr);
        mstr += m.symbol().c_str();
        STR_MARKUP_END_CURSIVE(mstr);
        mstr += "</a>";
        mstr += "<a name=\"";
        mstr += QString::number(result_parts.size());
        mstr += "\"></a>";
        break;
    }

    case STRUCT_POWER: {

        ips_n.depth++;

        ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);

        ips_n.division_depth++;
        mstr += drawStructure(m[0], po, ips_n);
        ips_n.division_depth--;

        ips_n.power_depth++;
        ips_n.wrap = m[1].needsParenthesis(po, ips_n, m, 2, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);

        PrintOptions po2 = po;
        po2.show_ending_zeroes = false;
        if (ips.power_depth > 0) {
            mstr += TEXT_TAGS "<sup>" "^" "</sup>" TEXT_TAGS_END;
            mstr += drawStructure(m[1], po2, ips_n);
        } else {
            mstr += drawStructure(m[1], po2, ips_n);
        }

        break;
    }

    case STRUCT_VARIABLE: {

        result_parts.push_back(m);
        mstr = "<a name=\"";
        mstr += QString::number(result_parts.size());
        mstr += "\">";
        if (m.variable() == CALCULATOR->v_i) {
            STR_MARKUP_BEGIN(mstr);
        } else {
            STR_MARKUP_BEGIN_CURSIVE(mstr);
        }

        QString str;
        const ExpressionName *ename = &m.variable()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names, po.can_display_unicode_string_function, po.can_display_unicode_string_arg);
        if (ename->suffix && ename->name.length() > 1) {
            size_t i = ename->name.rfind('_');
            bool b = i == string::npos || i == ename->name.length() - 1 || i == 0;
            size_t i2 = 1;
            if (b) {
                if (is_in(NUMBERS, ename->name[ename->name.length() - 1])) {
                    while (ename->name.length() > i2 + 1 && is_in(NUMBERS, ename->name[ename->name.length() - 1 - i2])) {
                        i2++;
                    }
                }
                str += ename->name.substr(0, ename->name.length() - i2).c_str();
            } else {
                str += ename->name.substr(0, i).c_str();
            }
            if (ips.power_depth == 0) str += "<sub>";
            else str += TEXT_TAGS_SMALL "<sup>";
            if (b) str += ename->name.substr(ename->name.length() - i2, i2).c_str();
            else str += ename->name.substr(i + 1, ename->name.length() - (i + 1)).c_str();
            if (ips.power_depth == 0) str += "</sub>";
            else str += "</sup>" TEXT_TAGS_SMALL_END;
        } else {
            str += ename->name.c_str();
        }
        str.replace('_', ' ');
        mstr += str;

        if (m.variable() == CALCULATOR->v_i) {
            STR_MARKUP_END(mstr);
        } else {
            STR_MARKUP_END_CURSIVE(mstr);
        }
        mstr += "</a>";
        /*result_parts.push_back(m);
              mstr += "<a name=\"";
              mstr += QString::number(result_parts.size());
              mstr += "\"></a>";*/
        break;
    }

    case STRUCT_MULTIPLICATION: {

        ips_n.depth++;

        QString mul_str;
        if (po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_MULTIDOT, po.can_display_unicode_string_arg))) {
            STR_MARKUP_ADD(mul_str, SIGN_MULTIDOT);
        } else if (po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_MULTIBULLET, po.can_display_unicode_string_arg))) {
            STR_MARKUP_ADD(mul_str, SIGN_MULTIBULLET);
        } else if (po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_SMALLCIRCLE, po.can_display_unicode_string_arg))) {
            STR_MARKUP_ADD(mul_str, SIGN_SMALLCIRCLE);
        } else if (po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) {
            STR_MARKUP_ADD(mul_str, SIGN_MULTIPLICATION);
        } else {
            STR_MARKUP_ADD(mul_str, QChar(0x22C5));
        }

        bool par_prev = false;
        vector<int> nm;
        vector<QString> terms;
        vector<bool> do_space;
        for (size_t i = 0; i < m.size(); i++) {
            ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
            terms.push_back(drawStructure(m[i], po, ips_n));
            if (!po.short_multiplication && i > 0) {
                nm.push_back(-1);
            } else if (i > 0) {
                nm.push_back(m[i].neededMultiplicationSign(po, ips_n, m, i + 1, ips_n.wrap || (m[i].isPower() && m[i][0].needsParenthesis(po, ips_n, m[i], 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0)), par_prev, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0));
            } else {
                nm.push_back(-1);
            }
            do_space.push_back(!terms[i].endsWith(QLatin1String("valign=\"middle\">")));
            par_prev = ips_n.wrap;
        }
        for (size_t i = 0; i < terms.size(); i++) {
            if (!po.short_multiplication && i > 0) {
                if (do_space[i - 1]) STR_MARKUP_ADD(mstr, " ");
                mstr += mul_str;
                if (do_space[i]) STR_MARKUP_ADD(mstr, " ");
            } else if (i > 0) {
                switch (nm[i]) {
                case MULTIPLICATION_SIGN_SPACE: {
                    if (do_space[i - 1] && do_space[i]) STR_MARKUP_ADD(mstr, " ");
                    break;
                }
                case MULTIPLICATION_SIGN_OPERATOR: {
                    if (do_space[i - 1]) STR_MARKUP_ADD(mstr, " ");
                    mstr += mul_str;
                    if (do_space[i]) STR_MARKUP_ADD(mstr, " ");
                    break;
                }
                case MULTIPLICATION_SIGN_OPERATOR_SHORT: {
                    mstr += mul_str;
                    break;
                }
                }
            }
            mstr += terms[i];
        }
        break;
    }

    case STRUCT_ADDITION: {

        ips_n.depth++;

        vector<QString> terms;
        vector<bool> do_space;
        for (size_t i = 0; i < m.size(); i++) {
            if (m[i].type() == STRUCT_NEGATE && i > 0) {
                ips_n.wrap = m[i][0].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
                terms.push_back(drawStructure(m[i][0], po, ips_n));
            } else {
                ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
                terms.push_back(drawStructure(m[i], po, ips_n));
            }
            do_space.push_back(!terms[i].endsWith(QLatin1String("valign=\"middle\">")));
        }
        for (size_t i = 0; i < terms.size(); i++) {
            if (i > 0) {
                if (do_space[i  - 1]) STR_MARKUP_ADD(mstr, " ");
                if (m[i].type() == STRUCT_NEGATE) {
                    if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_MINUS, po.can_display_unicode_string_arg))) {
                        STR_MARKUP_ADD(mstr, SIGN_MINUS);
                    } else {
                        STR_MARKUP_ADD(mstr, "-");
                    }
                } else {
                    STR_MARKUP_ADD(mstr, "+");
                }
                if (do_space[i]) STR_MARKUP_ADD(mstr, " ");
            }
            mstr += terms[i];
        }
        break;
    }

    case STRUCT_FUNCTION: {

        ips_n.depth++;

        result_parts.push_back(m);
        mstr = "<a name=\"";
        mstr += QString::number(result_parts.size());
        mstr += "\">";

        STR_MARKUP_BEGIN(mstr);

        QString str;
        const ExpressionName *ename = &m.function()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names, po.can_display_unicode_string_function, po.can_display_unicode_string_arg);
        if (ename->suffix && ename->name.length() > 1) {
            size_t i = ename->name.rfind('_');
            bool b = i == string::npos || i == ename->name.length() - 1 || i == 0;
            size_t i2 = 1;
            if (b) {
                if (is_in(NUMBERS, ename->name[ename->name.length() - 1])) {
                    while (ename->name.length() > i2 + 1 && is_in(NUMBERS, ename->name[ename->name.length() - 1 - i2])) {
                        i2++;
                    }
                }
                str += ename->name.substr(0, ename->name.length() - i2).c_str();
            } else {
                str += ename->name.substr(0, i).c_str();
            }
            if (ips.power_depth == 0) str += "<sub>";
            else str += TEXT_TAGS_SMALL "<sup>";
            if (b) str += ename->name.substr(ename->name.length() - i2, i2).c_str();
            else str += ename->name.substr(i + 1, ename->name.length() - (i + 1)).c_str();
            if (ips.power_depth == 0) str += "</sub>";
            else str += "</sup>" TEXT_TAGS_SMALL_END;
        } else {
            str += ename->name.c_str();
        }
        str.replace('_', ' ');
        mstr += str;

        mstr += '(';
        STR_MARKUP_END(mstr);
        mstr += "</a>";

        for (size_t index = 0; index < m.size(); index++) {
            if (index > 0) {
                STR_MARKUP_BEGIN(mstr);
                mstr += po.comma().c_str();
                mstr += ' ';
                STR_MARKUP_END(mstr);
            }
            mstr += drawStructure(m[index], po, ips_n);
        }
        STR_MARKUP_ADD(mstr, ")");

        break;
    }

    case STRUCT_UNIT: {

        result_parts.push_back(m);
        mstr = "<a name=\"";
        mstr += QString::number(result_parts.size());
        mstr += "\">";

        STR_MARKUP_BEGIN(mstr);

        QString str;
        const ExpressionName *ename = &m.unit()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, m.isPlural(), po.use_reference_names, po.can_display_unicode_string_function, po.can_display_unicode_string_arg);
        if (m.prefix()) {
            //str += m.prefix()->name(po.abbreviate_names && ename->abbreviation && (ename->suffix || ename->name.find("_") == string::npos), po.use_unicode_signs, po.can_display_unicode_string_function, po.can_display_unicode_string_arg).c_str();
            str += m.prefix()->name(po.abbreviate_names, po.use_unicode_signs, po.can_display_unicode_string_function, po.can_display_unicode_string_arg).c_str();
        }

        if (ename->suffix && ename->name.length() > 1) {
            size_t i = ename->name.rfind('_');
            bool b = i == string::npos || i == ename->name.length() - 1 || i == 0;
            size_t i2 = 1;
            if (b) {
                if (is_in(NUMBERS, ename->name[ename->name.length() - 1])) {
                    while (ename->name.length() > i2 + 1 && is_in(NUMBERS, ename->name[ename->name.length() - 1 - i2])) {
                        i2++;
                    }
                }
                str += ename->name.substr(0, ename->name.length() - i2).c_str();
            } else {
                str += ename->name.substr(0, i).c_str();
            }
            if (ips.power_depth == 0) str += "<sub>";
            else str += TEXT_TAGS_SMALL "<sup>";
            if (b) str += ename->name.substr(ename->name.length() - i2, i2).c_str();
            else str += ename->name.substr(i + 1, ename->name.length() - (i + 1)).c_str();
            if (ips.power_depth == 0) str += "</sub>";
            else str += "</sup>" TEXT_TAGS_SMALL_END;
        } else {
            if (ename->name == "ohm" || ename->name == "ohms")
                str += QChar(0x2126);
            else if (ename->name == "oF")
                str += QChar(0x2109);
            else if (ename->name == "oC")
                str += QChar(0x2103);
            else if (ename->name == "GBP")
                str += QChar(0xA3);
            else if (ename->name == "JPY")
                str += QChar(0xA5);
            else if (ename->name == "EUR")
                str += QChar(0x20AC);
            else if (ename->name == "USD")
                str += QChar(0x24);
            else
                str += ename->name.c_str();
        }
        str.replace('_', ' ');
        mstr += str;

        STR_MARKUP_END(mstr);
        mstr += "</a>";
        mstr += "<a name=\"";
        mstr += QString::number(result_parts.size());
        mstr += "\"></a>";

        break;
    }

    case STRUCT_INVERSE: {}
    case STRUCT_DIVISION: {

        ips_n.depth++;
        ips_n.division_depth++;

        bool flat = ips.division_depth > 0 || ips.power_depth > 0;
        flat = true;
        if (!flat && po.place_units_separately) {
            flat = true;
            size_t i = 0;
            if (m.isDivision()) {
                i = 1;
            }
            if (m[i].isMultiplication()) {
                for (size_t i2 = 0; i2 < m[i].size(); i2++) {
                    if (!m[i][i2].isUnit_exp()) {
                        flat = false;
                        break;
                    }
                }
            } else if (!m[i].isUnit_exp()) {
                flat = false;
            }
            if (flat) {
                ips_n.division_depth--;
            }
        }
        QString num_str, den_str;
        if (m.type() == STRUCT_DIVISION) {
            ips_n.wrap = (!m[0].isDivision() || !flat || ips.division_depth > 0 || ips.power_depth > 0) && m[0].needsParenthesis(po, ips_n, m, 1, flat, ips.power_depth > 0);
            num_str = drawStructure(m[0], po, ips_n);
        } else {
            MathStructure onestruct(1, 1);
            ips_n.wrap = false;
            num_str = drawStructure(onestruct, po, ips_n);
        }
        if (m.type() == STRUCT_DIVISION) {
            ips_n.wrap = m[1].needsParenthesis(po, ips_n, m, 2, flat, ips.power_depth > 0);
            den_str = drawStructure(m[1], po, ips_n);
        } else {
            ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 2, flat, ips.power_depth > 0);
            den_str = drawStructure(m[0], po, ips_n);
        }

        if (flat) {
            QString div_str;
            if (po.use_unicode_signs && po.division_sign == DIVISION_SIGN_DIVISION && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_DIVISION, po.can_display_unicode_string_arg))) {
                STR_MARKUP_ADD(div_str, " " SIGN_DIVISION " ");
            } else if (po.use_unicode_signs && po.division_sign == DIVISION_SIGN_DIVISION_SLASH && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_DIVISION_SLASH, po.can_display_unicode_string_arg))) {
                STR_MARKUP_ADD(div_str, " " SIGN_DIVISION_SLASH " ");
            } else {
                STR_MARKUP_ADD(div_str, " / ");
            }
            mstr = num_str;
            mstr += div_str;
            mstr += den_str;
        }
//          else {
//
//             int den_w, num_w, w = 0, h = 0;
//
//             num_str.prepend(TEXT_TAGS);
//             num_str += TEXT_TAGS_END;
//             den_str.prepend(TEXT_TAGS);
//             den_str += TEXT_TAGS_END;
//
//             QSimpleRichText text_r(num_str, font); text_r.setWidth(10000);
//             num_w = text_r.widthUsed();
//             QSimpleRichText text_rn(den_str, font); text_rn.setWidth(10000);
//             den_w = text_rn.widthUsed();
//
//             w = den_w;
//             if(num_w > w) w = num_w;
//             w += 2;
//
//             QSimpleRichText textsmall("<font size=\"1\">X</font>", font);
//             h = (int) (textsmall.height() / 1.5);
//             h += h % 2;
//
//             string filename = getLocalDir();
//             if(saved_divisionline_height != h) {
//
//                QPixmap *pixmap = new QPixmap(10, h);
//                pixmap->fill(cg.background());
//                QPainter p(pixmap);
//                QPen ppen = p.pen();
//                ppen.setColor(cg.foreground());
//                p.setPen(ppen);
//                p.drawLine(0, h / 2, 10, h / 2);
//                p.drawLine(0, h / 2 + 1, 10, h / 2 + 1);
//                p.flush();
//                p.end();
//
//                pixmap->setMask(pixmap->createHeuristicMask());
//                mkdir(filename.c_str(), S_IRWXU);
//                filename += "tmp/";
//                mkdir(filename.c_str(), S_IRWXU);
//                filename += "divline.png";
//                pixmap->save(filename.c_str(), "PNG", 100);
//
//                delete pixmap;
//
//                saved_divisionline_height = h;
//
//             } else {
//
//                filename += "tmp/divline.png";
//
//             }
//
//             mstr = TEXT_TAGS_END;
//             mstr = "</td><td width=1 align=\"center\" valign=\"middle\">";
//             mstr += num_str;
//             mstr += "<br><font size=1><img align=\"middle\" width=";
//             mstr += QString::number(w);
//             mstr += " height=";
//             mstr += QString::number(h);
//             mstr += " src=\"";
//             mstr += filename.c_str();
//             mstr += "\"><br></font>";
//             mstr += den_str;
//             mstr += "</td><td width=1 align=\"center\" valign=\"middle\">";
//             mstr += TEXT_TAGS;
//
//          }
        break;
    }

    case STRUCT_NEGATE: {

        ips_n.depth++;

        ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
        if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_MINUS, po.can_display_unicode_string_arg))) {
            STR_MARKUP_ADD(mstr, SIGN_MINUS);
        } else {
            STR_MARKUP_ADD(mstr, "-");
        }
        mstr += drawStructure(m[0], po, ips_n);
        break;
    }

    case STRUCT_VECTOR: {

        ips_n.depth++;

//         bool is_matrix = m.isMatrix();
        if (!in_matrix) {
            result_parts.push_back(m);
            mstr = "<a name=\"";
            mstr += QString::number(result_parts.size());
            mstr += "\">";
        }

        if (m.size() == 0) {
            STR_MARKUP_ADD(mstr, "( ");
        } else {
            STR_MARKUP_ADD(mstr, "(");
        }
        for (size_t index = 0; index < m.size(); index++) {
            if (index > 0) {
                STR_MARKUP_BEGIN(mstr);
                mstr += CALCULATOR->getComma().c_str();
                mstr += ' ';
                STR_MARKUP_END(mstr);
            }
            ips_n.wrap = m[index].needsParenthesis(po, ips_n, m, index + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
            mstr += drawStructure(m[index], po, ips_n);
        }
        STR_MARKUP_ADD(mstr, ")");
        if (!in_matrix) mstr += "</a>";
        break;
    }

    case STRUCT_LOGICAL_AND: {
        if (!po.preserve_format && m.size() == 2 && m[0].isComparison() && m[1].isComparison() && m[0].comparisonType() != COMPARISON_EQUALS && m[0].comparisonType() != COMPARISON_NOT_EQUALS && m[1].comparisonType() != COMPARISON_EQUALS && m[1].comparisonType() != COMPARISON_NOT_EQUALS && m[0][0] == m[1][0]) {

            ips_n.depth++;

            bool do_space = true;
            QString tstr;
            ips_n.wrap = m[0][1].needsParenthesis(po, ips_n, m[0], 2, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
            tstr = drawStructure(m[0][1], po, ips_n);
            do_space = !tstr.endsWith(QLatin1String("valign=\"middle\">"));
            mstr += tstr;
            STR_MARKUP_BEGIN(mstr);
            if (do_space) mstr += ' ';
            switch (m[0].comparisonType()) {
            case COMPARISON_LESS: {
                mstr += "&gt;";
                break;
            }
            case COMPARISON_GREATER: {
                mstr += "&lt;";
                break;
            }
            case COMPARISON_EQUALS_LESS: {
                if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_GREATER_OR_EQUAL, po.can_display_unicode_string_arg))) {
                    mstr += SIGN_GREATER_OR_EQUAL;
                } else {
                    mstr += "&gt;=";
                }
                break;
            }
            case COMPARISON_EQUALS_GREATER: {
                if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_LESS_OR_EQUAL, po.can_display_unicode_string_arg))) {
                    mstr += SIGN_LESS_OR_EQUAL;
                } else {
                    mstr += "&lt;=";
                }
                break;
            }
            default: {}
            }

            ips_n.wrap = m[0][0].needsParenthesis(po, ips_n, m[0], 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
            tstr = drawStructure(m[0][0], po, ips_n);
            do_space = !tstr.endsWith(QLatin1String("valign=\"middle\">"));
            if (do_space) mstr += ' ';
            STR_MARKUP_END(mstr);
            mstr += tstr;
            STR_MARKUP_BEGIN(mstr);
            if (do_space) mstr += ' ';

            switch (m[1].comparisonType()) {
            case COMPARISON_GREATER: {
                mstr += "&gt;";
                break;
            }
            case COMPARISON_LESS: {
                mstr += "&lt;";
                break;
            }
            case COMPARISON_EQUALS_GREATER: {
                if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_GREATER_OR_EQUAL, po.can_display_unicode_string_arg))) {
                    mstr += SIGN_GREATER_OR_EQUAL;
                } else {
                    mstr += "&gt;=";
                }
                break;
            }
            case COMPARISON_EQUALS_LESS: {
                if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_LESS_OR_EQUAL, po.can_display_unicode_string_arg))) {
                    mstr += SIGN_LESS_OR_EQUAL;
                } else {
                    mstr += "&lt;=";
                }
                break;
            }
            default: {}
            }

            ips_n.wrap = m[1][1].needsParenthesis(po, ips_n, m[1], 2, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
            tstr = drawStructure(m[1][1], po, ips_n);
            do_space = !tstr.endsWith(QLatin1String("valign=\"middle\">"));
            if (do_space) mstr += ' ';
            STR_MARKUP_END(mstr);
            mstr += tstr;

            break;
        }
    }
    case STRUCT_COMPARISON: {}
    case STRUCT_LOGICAL_XOR: {}
    case STRUCT_LOGICAL_OR: {}
    case STRUCT_BITWISE_AND: {}
    case STRUCT_BITWISE_XOR: {}
    case STRUCT_BITWISE_OR: {

        ips_n.depth++;

        QString str;
        if (m.type() == STRUCT_COMPARISON) {
            switch (m.comparisonType()) {
            case COMPARISON_EQUALS: {
                break;
            }
            case COMPARISON_NOT_EQUALS: {
                if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_NOT_EQUAL, po.can_display_unicode_string_arg))) {
                    str += SIGN_NOT_EQUAL;
                } else {
                    str += "!=";
                }
                break;
            }
            case COMPARISON_GREATER: {
                str += "&gt;";
                break;
            }
            case COMPARISON_LESS: {
                str += "&lt;";
                break;
            }
            case COMPARISON_EQUALS_GREATER: {
                if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_GREATER_OR_EQUAL, po.can_display_unicode_string_arg))) {
                    str += SIGN_GREATER_OR_EQUAL;
                } else {
                    str += "&gt;=";
                }
                break;
            }
            case COMPARISON_EQUALS_LESS: {
                if (po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_LESS_OR_EQUAL, po.can_display_unicode_string_arg))) {
                    str += SIGN_LESS_OR_EQUAL;
                } else {
                    str += "&lt;=";
                }
                break;
            }
            }
        } else if (m.type() == STRUCT_LOGICAL_AND) {
            str += "<a name=\"+Logical AND\">";
            if (po.spell_out_logical_operators) str += "and";
            else str += "&amp;&amp;";
            str += "</a>";
        } else if (m.type() == STRUCT_LOGICAL_OR) {
            str += "<a name=\"+Logical inclusive OR\">";
            if (po.spell_out_logical_operators) str += "or";
            else str += "||";
            str += "</a>";
        } else if (m.type() == STRUCT_LOGICAL_XOR) {
            str += "<a name=\"+Logical exclusive OR\">XOR</a>";
        } else if (m.type() == STRUCT_BITWISE_AND) {
            str += "<a name=\"+Bitwise AND\">&amp;</a>";
        } else if (m.type() == STRUCT_BITWISE_OR) {
            str += "<a name=\"+Bitwise inclusive OR\">|</a>";
        } else if (m.type() == STRUCT_BITWISE_XOR) {
            str += "<a name=\"+Bitwise exclusive XOR\">XOR</a>";
        }

        bool do_space = true, do_space_prev = true;
        QString tstr;
        for (size_t i = 0; i < m.size(); i++) {
            do_space_prev = do_space;
            ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
            tstr = drawStructure(m[i], po, ips_n);
            do_space = !tstr.endsWith(QLatin1String("valign=\"middle\">"));
            if (i > 0) {
                STR_MARKUP_BEGIN(mstr);
                if (do_space_prev) mstr += ' ';
                if (m.isComparison() && m.comparisonType() == COMPARISON_EQUALS) {
                    if (ips.depth == 0 && po.use_unicode_signs && (*po.is_approximate || m.isApproximate()) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function)(SIGN_ALMOST_EQUAL, po.can_display_unicode_string_arg))) {
                        mstr += SIGN_ALMOST_EQUAL;
                    } else {
                        mstr += '=';
                    }
                } else {
                    mstr += str;
                }
                if (do_space) mstr += ' ';
                STR_MARKUP_END(mstr);
            }
            mstr += tstr;
        }
        break;
    }
    case STRUCT_BITWISE_NOT: {

        ips_n.depth++;

        STR_MARKUP_BEGIN(mstr);

        mstr += "<a name=\"+Bitwise NOT\">~</a>";

        ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
        mstr += drawStructure(m[0], po, ips_n);

        STR_MARKUP_END(mstr);

        break;
    }
    case STRUCT_LOGICAL_NOT: {

        ips_n.depth++;

        STR_MARKUP_BEGIN(mstr);

        mstr += "<a name=\"+Logical NOT\">!</a>";

        ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
        mstr += drawStructure(m[0], po, ips_n);

        STR_MARKUP_END(mstr);

        break;
    }
    case STRUCT_UNDEFINED: {
        STR_MARKUP_ADD(mstr, "undefined");
        break;
    }
    }

    if (ips.wrap) {
        STR_MARKUP_PREPEND(mstr, "(");
        STR_MARKUP_ADD(mstr, ")");
    }

    if (ips.depth == 0) {
        mstr.prepend(TEXT_TAGS);
        mstr += TEXT_TAGS_END;
    }

    return mstr;
}
