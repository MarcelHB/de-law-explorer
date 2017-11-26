/* SPDX-License-Identifier: GPL-3.0 */

#ifndef GLWIDGET_H
#define GLWIDGET_H

#ifdef USE_GLEW_NOT_QT
#include "GL/glew.h"
#endif

#include <QGLWidget>

#include "scene/scene_manager.h"
#include "scene/simple_scene_writer.h"
#include "../controllers/controller.h"

class GLWidget : public QGLWidget {
    Q_OBJECT
  public:
    explicit GLWidget(QWidget *parent = 0);
    ~GLWidget();

    void invalidate_scene();
    void launch_controller(Controller&, const ControllerArgs&);
  signals:
    void element_selected(SceneElement&);
  protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int, int);
    void keyPressEvent(QKeyEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);

  private slots:
    void on_element_selected_int(SceneElement&);
  private:
    SceneManager sm;
    SceneWriter *current_sw;
    bool scene_loaded;
    QMetaObject::Connection con_scene_element_selected;
    enum { NONE, DOWN, MOVED } mouse_state;
    int last_mouse_pos[2];
};

#endif // GLWIDGET_H
