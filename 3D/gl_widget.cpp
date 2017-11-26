/* SPDX-License-Identifier: GPL-3.0 */

#include <fstream>
#include <cmath>
#include <vector>

#include <QKeyEvent>
#include <QTimer>

#define USE_GLEW_NOT_QT
#include "gl_widget.h"

#include "glm/glm.hpp"

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(parent),
    current_sw(nullptr),
    scene_loaded(false),
    mouse_state(NONE)
{
    this->setFocusPolicy(Qt::StrongFocus);
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(17);

    QGLFormat format;
    QGLContext* ctx = new QGLContext(format, this);

    format.setSampleBuffers(true);
    format.setSamples(2);

    ctx->setFormat(format);
    this->setContext(ctx);

    this->con_scene_element_selected = connect(&this->sm, SIGNAL(element_selected(SceneElement&)),
                                               this, SLOT(on_element_selected_int(SceneElement&)));
}

GLWidget::~GLWidget()
{
    disconnect(this->con_scene_element_selected);
}

void GLWidget::launch_controller(Controller &ctrl, const ControllerArgs &args) {
    if(this->current_sw != nullptr) {
        SceneWriter *next_sw = ctrl.propose_scene_writer(args, this->sm);

        if(!(*this->current_sw == *next_sw)) {
            ctrl.gl_action();
        } else {
            next_sw = ctrl.continue_scene_writer(this->current_sw);
        }
        this->current_sw = next_sw;
    } else {
        this->current_sw = ctrl.propose_scene_writer(args, this->sm);
        ctrl.gl_action();
    }
}

void GLWidget::invalidate_scene() {
    this->current_sw = nullptr;
}

void GLWidget::initializeGL() {
    glewExperimental = GL_TRUE;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    this->sm.update_viewport(this->x(), this->y(),
                             this->width(), this->height());
}

void GLWidget::paintGL() {
    this->setFocus();
    this->sm.frame();
}

void GLWidget::resizeGL(int width, int height) {
    QGLWidget::resizeGL(width, height);
    this->sm.update_viewport(this->x(), this->y(), width, height);
}

void GLWidget::mouseMoveEvent(QMouseEvent *e) {
    if(this->mouse_state == DOWN || this->mouse_state == MOVED) {
        this->mouse_state = MOVED;
        this->setCursor(Qt::ClosedHandCursor);

        glm::vec3 movement((e->x() - this->last_mouse_pos[0]) / -100.0,
                           0,
                           (e->y() - this->last_mouse_pos[1]) / -100.0);

        this->sm.move_camera(movement);
        glm::vec3 look_at = this->sm.look_at();
        look_at += movement;
        this->sm.look_at(look_at);

        this->last_mouse_pos[0] = e->x();
        this->last_mouse_pos[1] = e->y();
    }
}

void GLWidget::mousePressEvent(QMouseEvent *e) {
    if(this->mouse_state == NONE) {
        this->mouse_state = DOWN;
        this->last_mouse_pos[0] = e->x();
        this->last_mouse_pos[1] = e->y();
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *e) {
    if(this->mouse_state == DOWN) {
        this->mouse_state = NONE;
        this->sm.select_element(e->x(), e->y());
    } else if(this->mouse_state == MOVED) {
        this->mouse_state = NONE;
    }
    this->setCursor(Qt::ArrowCursor);
}

void GLWidget::keyPressEvent(QKeyEvent *e) {
    QGLWidget::keyPressEvent(e);
}

void GLWidget::wheelEvent(QWheelEvent *e) {
    glm::vec3 dir = this->sm.look_at();
    glm::vec3 cam = this->sm.camera();
    /* Simple wheel roll delta is 120 here, with neg. being pull back. */
    glm::vec3 y_off(0, (e->delta() / 100.0f), 0);
    dir += y_off; cam += y_off;
    this->sm.look_at(dir);
    this->sm.camera(cam);
}

void GLWidget::on_element_selected_int(SceneElement &elem) {
    emit this->element_selected(elem);
}
