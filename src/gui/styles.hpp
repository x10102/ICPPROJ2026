/**
 * @file styles.hpp
 * @author Ondřej Turek, xtureko00
 * @brief Qt style constants
 */
#ifndef _STYLES_HPP
#define _STYLES_HPP

#include <QString>
#include <qchar.h>


namespace styles {
    const QString SimPanelCustom =  "QFrame { background: %1; border-radius: 6px; }"
                                    "QPushButton { color: %2; min-width: 60px; padding: 4px 12px; }"
                                    "QPushButton:checked { background: #4a90d9; color: white; border-radius: 3px; }";

    const QString ToolPanelCustom = "QFrame { background: %1; border-radius: 6px; }"
                                    "QPushButton { color: %2; min-width: 90px; padding: 4px 8px; }"
                                    "QPushButton:checked { background: #4a90d9; color: white; border-radius: 3px; }";

    const QString ToolPanelDefault =    "QFrame { background: rgba(245,245,245,220); border-radius: 6px; }"
                                        "QPushButton { min-width: 90px; padding: 4px 8px; }"
                                        "QPushButton:checked { background: #4a90d9; color: white; border-radius: 3px; }";

    const QString SimPanelDefault = "QFrame { background: rgba(245,245,245,220); border-radius: 6px; }"
                                    "QPushButton { min-width: 60px; padding: 4px 12px; }"
                                    "QPushButton:checked { background: #4a90d9; color: white; border-radius: 3px; }";
}

#endif