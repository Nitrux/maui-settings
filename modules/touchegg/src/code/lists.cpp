/**************************************************************************
 * Copyright (c) 2012-2015 Raffaele Pertile <raffarti@zoho.com>
 * This file is part of touchegg-gce.
 *
 * touchegg-gce is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * touchegg-gce is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with touchegg-gce.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "lists.h"
#include <QDebug>
Lists::Lists(QObject *parent) :
    QObject(parent){}

//this class is an extremly boring one, just don't look at it.
QString Lists::toString(ActionType t){
    switch(t){
    case (NO_ACTION):
        return QString(tr("No Action"));
    case (MOUSE_CLICK):
        return QString(tr("Mouse Click"));
    case (SCROLL):
        return QString(tr("Scroll"));
    case (MINIMIZE_WINDOW):
        return QString(tr("Minimize"));
    case (MAXIMIZE_RESTORE_WINDOW):
        return QString(tr("Maximize"));
    case (CLOSE_WINDOW):
        return QString(tr("Close"));
    case (RESIZE_WINDOW):
        return QString(tr("Resize"));
    case (SHOW_DESKTOP):
        return QString(tr("Show Desktop"));
    case (CHANGE_DESKTOP):
        return QString(tr("Change Desktop"));
    case (CHANGE_VIEWPORT):
        return QString(tr("Change Viewport"));
    case (SEND_KEYS):
        return QString(tr("Send Keys"));
    case (MOVE_WINDOW):
        return QString(tr("Move Window"));
    case (RUN_COMMAND):
        return QString(tr("Run Command"));
    case (DRAG_AND_DROP):
        return QString(tr("Drag&Drop"));
    }
    return QString();
}


QString Lists::toString(GestureType t){
    switch(t){
    case (TAP):
        return QString(tr("Tap"));
    case (DRAG):
        return QString(tr("Drag"));
    case (PINCH):
        return QString(tr("Pinch"));
    case (ROTATE):
        return QString(tr("Rotate"));
    case (TAP_AND_HOLD):
        return QString(tr("Tap&Hold"));
    case (DOUBLE_TAP):
        return QString(tr("DoubleTap"));
    }
    return QString();
}


QString Lists::toString(GestureDirection t){
    switch(t){
    case (NO_DIRECTION):
        return QString("");
    case (ALL):
        return QString(tr("All"));
    case (UP):
        return QString(tr("Up"));
    case (DOWN):
        return QString(tr("Down"));
    case (LEFT):
        return QString(tr("Left"));
    case (RIGHT):
        return QString(tr("Right"));
    case (IN):
        return QString(tr("Inward"));
    case (OUT):
        return QString(tr("Outward"));
    }
    return QString();
}

QStringList Lists::actionList(){
    QStringList ret;
    for (int i = 0; !toString((ActionType)i).isNull();i++)
        ret.append(toString((ActionType)i));
    return ret;
}

QStringList Lists::gestureTypes(){
    QStringList ret;
    for (int i = 0; !toString((GestureType)i).isNull();i++)
        ret.append(toString((GestureType)i));
    return ret;
}

QStringList Lists::gestureDirections(GestureType type){
    QStringList ret;
    for (int i = 0; !toString((GestureDirection)i).isNull();i++)
        if(got(type,(GestureDirection)i))ret.append(toString((GestureDirection)i));
    return ret;
}

Lists::ActionType Lists::toAT(QString s){
    if (s == tr("No Action"))
        return NO_ACTION;
    else if (s == tr("Mouse Click"))
        return MOUSE_CLICK;
    else if (s == tr("Scroll"))
        return SCROLL;
    else if (s == tr("Minimize"))
        return MINIMIZE_WINDOW;
    else if (s == tr("Maximize"))
        return MAXIMIZE_RESTORE_WINDOW;
    else if (s == tr("Close"))
        return CLOSE_WINDOW;
    else if (s == tr("Resize"))
        return RESIZE_WINDOW;
    else if (s == tr("Show Desktop"))
        return SHOW_DESKTOP;
    else if (s == tr("Change Desktop"))
        return CHANGE_DESKTOP;
    else if (s == tr("Change Viewport"))
        return CHANGE_VIEWPORT;
    else if (s == tr("Send Keys"))
        return SEND_KEYS;
    else if (s == tr("Move Window"))
        return MOVE_WINDOW;
    else if (s == tr("Run Command"))
        return RUN_COMMAND;
    else if (s == tr("Drag&Drop"))
        return DRAG_AND_DROP;
    qDebug() << QString("toAT: String \"%1\" not reconized.").arg(s);
    return NO_ACTION;
}

Lists::GestureType Lists::gT(QString s){
    if (s == "TAP")
        return TAP;
    else if (s == "DRAG")
        return DRAG;
    else if (s == "PINCH")
        return PINCH;
    else if (s == "ROTATE")
        return ROTATE;
    else if (s == "TAP_AND_HOLD")
        return TAP_AND_HOLD;
    else if (s == "DOUBLE_TAP")
        return DOUBLE_TAP;
    qDebug() << QString("gT: TAG \"%1\" not reconized.").arg(s);
    return (GestureType)-1;
}

Lists::GestureDirection Lists::toGD(QString s){
    if (s == "")
        return NO_DIRECTION;
    else if (s == tr("All"))
        return ALL;
    else if (s == tr("Up"))
        return UP;
    else if (s == tr("Down"))
        return DOWN;
    else if (s == tr("Left"))
        return LEFT;
    else if (s == tr("Right"))
        return RIGHT;
    else if (s == tr("Inward"))
        return IN;
    else if (s == tr("Outward"))
        return OUT;
    qDebug() << QString("toGD: String \"%1\" not reconized.").arg(s);
    return (GestureDirection)-1;
}

Lists::GestureDirection Lists::gD(QString s){
    if (s == "")
        return NO_DIRECTION;
    else if (s == "ALL")
        return ALL;
    else if (s == "UP")
        return UP;
    else if (s == "DOWN")
        return DOWN;
    else if (s == "LEFT")
        return LEFT;
    else if (s == "RIGHT")
        return RIGHT;
    else if (s == "IN")
        return IN;
    else if (s == "OUT")
        return (OUT);
    qDebug() << QString("gD: String \"%1\" not reconized.").arg(s);
    return (GestureDirection)-1;
}

Lists::GestureType Lists::toGT(QString s){
    if (s == tr("Tap"))
        return TAP;
    else if (s == tr("Drag"))
        return DRAG;
    else if (s == tr("Pinch"))
        return PINCH;
    else if (s == tr("Rotate"))
        return ROTATE;
    else if (s == tr("Tap&Hold"))
        return TAP_AND_HOLD;
    else if (s == tr("DoubleTap"))
        return DOUBLE_TAP;
    qDebug() << QString("toGT: String \"%1\" not reconized.").arg(s);
    return (GestureType)-1;
}

Lists::ActionType Lists::aT(QString s){
    if (s == "NO_ACTION")
        return NO_ACTION;
    else if (s == "MOUSE_CLICK")
        return MOUSE_CLICK;
    else if (s == "SCROLL")
        return SCROLL;
    else if (s == "MINIMIZE_WINDOW")
        return MINIMIZE_WINDOW;
    else if (s == "MAXIMIZE_RESTORE_WINDOW")
        return MAXIMIZE_RESTORE_WINDOW;
    else if (s == "CLOSE_WINDOW")
        return CLOSE_WINDOW;
    else if (s == "RESIZE_WINDOW")
        return RESIZE_WINDOW;
    else if (s == "SHOW_DESKTOP")
        return SHOW_DESKTOP;
    else if (s == "CHANGE_DESKTOP")
        return CHANGE_DESKTOP;
    else if (s == "CHANGE_VIEWPORT")
        return CHANGE_VIEWPORT;
    else if (s == "SEND_KEYS")
        return SEND_KEYS;
    else if (s == "MOVE_WINDOW")
        return MOVE_WINDOW;
    else if (s == "RUN_COMMAND")
        return RUN_COMMAND;
    else if (s == "DRAG_AND_DROP")
        return DRAG_AND_DROP;
    qDebug() << QString("aT: TAG \"%1\" not reconized.").arg(s);
    return (ActionType)-1;
}

QString Lists::trnAT(QString s){
    if (s == "NO_ACTION")
        return toString(NO_ACTION);
    else if (s == "MOUSE_CLICK")
        return toString(MOUSE_CLICK);
    else if (s == "SCROLL")
        return toString(SCROLL);
    else if (s == "MINIMIZE_WINDOW")
        return toString(MINIMIZE_WINDOW);
    else if (s == "MAXIMIZE_RESTORE_WINDOW")
        return toString(MAXIMIZE_RESTORE_WINDOW);
    else if (s == "CLOSE_WINDOW")
        return toString(CLOSE_WINDOW);
    else if (s == "RESIZE_WINDOW")
        return toString(RESIZE_WINDOW);
    else if (s == "SHOW_DESKTOP")
        return toString(SHOW_DESKTOP);
    else if (s == "CHANGE_DESKTOP")
        return toString(CHANGE_DESKTOP);
    else if (s == "CHANGE_VIEWPORT")
        return toString(CHANGE_VIEWPORT);
    else if (s == "SEND_KEYS")
        return toString(SEND_KEYS);
    else if (s == "MOVE_WINDOW")
        return toString(MOVE_WINDOW);
    else if (s == "RUN_COMMAND")
        return toString(RUN_COMMAND);
    else if (s == "DRAG_AND_DROP")
        return toString(DRAG_AND_DROP);
    qDebug() << QString("trnAT: TAG \"%1\" not reconized.").arg(s);
    return toString(NO_ACTION);
}

QString Lists::trnGT(QString s){
    if (s == "TAP")
        return toString(TAP);
    else if (s == "DRAG")
        return toString(DRAG);
    else if (s == "PINCH")
        return toString(PINCH);
    else if (s == "ROTATE")
        return toString(ROTATE);
    else if (s == "TAP_AND_HOLD")
        return toString(TAP_AND_HOLD);
    else if (s == "DOUBLE_TAP")
        return toString(DOUBLE_TAP);
    qDebug() << QString("trnGT: TAG \"%1\" not reconized.").arg(s);
    return QString();
}

QString Lists::trnGD(QString s){
    if (s == "")
        return toString(NO_DIRECTION);
    else if (s == "ALL")
        return toString(ALL);
    else if (s == "DOWN")
        return toString(DOWN);
    else if (s == "UP")
        return toString(UP);
    else if (s == "LEFT")
        return toString(LEFT);
    else if (s == "RIGHT")
        return toString(RIGHT);
    else if (s == "IN")
        return toString(IN);
    else if (s == "OUT")
        return toString(OUT);
    qDebug() << QString("trnGD: TAG \"%1\" not reconized.").arg(s);
    return QString();
}

bool Lists::got(GestureType type, GestureDirection dir){
    switch(type){
    case TAP:;
    case DOUBLE_TAP: return dir == NO_DIRECTION;
    case PINCH: return dir == ALL || dir == IN || dir == OUT;
    case ROTATE: return dir == ALL || dir == LEFT || dir == RIGHT;
    case TAP_AND_HOLD:;
    case DRAG: return dir == ALL || dir == LEFT || dir == RIGHT || dir == UP || dir == DOWN;
    default: return false;
    }
    return false;
}


QString Lists::printTag(ActionType t){
    switch(t){
    case (NO_ACTION):
        return QString("NO_ACTION");
    case (MOUSE_CLICK):
        return QString("MOUSE_CLICK");
    case (SCROLL):
        return QString("SCROLL");
    case (MINIMIZE_WINDOW):
        return QString("MINIMIZE_WINDOW");
    case (MAXIMIZE_RESTORE_WINDOW):
        return QString("MAXIMIZE_RESTORE_WINDOW");
    case (CLOSE_WINDOW):
        return QString("CLOSE_WINDOW");
    case (RESIZE_WINDOW):
        return QString("RESIZE_WINDOW");
    case (SHOW_DESKTOP):
        return QString("SHOW_DESKTOP");
    case (CHANGE_DESKTOP):
        return QString("CHANGE_DESKTOP");
    case (CHANGE_VIEWPORT):
        return QString("CHANGE_VIEWPORT");
    case (SEND_KEYS):
        return QString("SEND_KEYS");
    case (MOVE_WINDOW):
        return QString("MOVE_WINDOW");
    case (RUN_COMMAND):
        return QString("RUN_COMMAND");
    case (DRAG_AND_DROP):
        return QString("DRAG_AND_DROP");
    }
    return QString();
}


QString Lists::printTag(GestureType t){
    switch(t){
    case (TAP):
        return QString("TAP");
    case (DRAG):
        return QString("DRAG");
    case (PINCH):
        return QString("PINCH");
    case (ROTATE):
        return QString("ROTATE");
    case (TAP_AND_HOLD):
        return QString("TAP_AND_HOLD");
    case (DOUBLE_TAP):
        return QString("DOUBLE_TAP");
    }
    return QString();
}


QString Lists::printTag(GestureDirection t){
    switch(t){
    case (NO_DIRECTION):
        return QString("");
    case (ALL):
        return QString("ALL");
    case (UP):
        return QString("UP");
    case (DOWN):
        return QString("DOWN");
    case (LEFT):
        return QString("LEFT");
    case (RIGHT):
        return QString("RIGHT");
    case (IN):
        return QString("IN");
    case (OUT):
        return QString("OUT");
    }
    return QString();
}
