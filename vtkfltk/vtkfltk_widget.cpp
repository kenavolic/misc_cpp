
#include "vtkfltk_widget.h"

// FLTK
#include <FL/x.H>
// vtk
#include <vtkCommand.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyle.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkPickingManager.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkVersion.h>

namespace tailor::ks
{

  VtkFLTKWidget::VtkFLTKWidget(int lx, int ly, int lw, int lh, const char* ll)
    : Fl_Gl_Window(lx, ly, lw, lh, ll), vtkRenderWindowInteractor()
  {
    auto inStyle = vtkSmartPointer<vtkInteractorStyleSwitch>::New();
    inStyle->SetCurrentStyleToTrackballCamera();

    this->SetInteractorStyle(inStyle);
    this->end();
  }

  VtkFLTKWidget::~VtkFLTKWidget()
  {
    if (parent())
    {
      ((Fl_Group*)parent())->remove(*(Fl_Gl_Window*)this);
    }
  }

  VtkFLTKWidget* VtkFLTKWidget::New() { return nullptr; }

  void VtkFLTKWidget::makeReadyForFirstRender()
  {
    if (!m_isReadyForRendering)
    {
      assert((void*)fl_xid(this) != nullptr &&
             "makeReadyForFirstRender not called at the right time");
      assert(fl_display != nullptr &&
             "makeReadyForFirstRender not called at the right time");
      RenderWindow->SetWindowId((void*)fl_xid(this));
      RenderWindow->SetDisplayId(fl_display);
      m_isReadyForRendering = true;
    }
  }

  void VtkFLTKWidget::resetCamera()
  {
    RenderWindow->GetRenderers()->GetFirstRenderer()->ResetCamera();
    this->draw();
  }

  void VtkFLTKWidget::checkState() const
  {
    // std::cout << "fl id = " << (void *)fl_xid(this) << std::endl;
    // std::cout << "display id = " << fl_display << std::endl;
  }

  void VtkFLTKWidget::Initialize()
  {
    if (!RenderWindow)
    {
      vtkErrorMacro( << "VtkFLTKWidget::Initialize has no render window");
      return;
    }

    int* lsize = RenderWindow->GetSize();
    // enable everything and start rendering
    Enable();

    // We should NOT call ->Render yet, as it's entirely possible that
    // Initialize() is called before there's a valid Fl_Gl_Window!
    // RenderWindow->Render();

    // set the size in the render window interactor
    Size[0] = lsize[0];
    Size[1] = lsize[1];

    // this is initialized
    Initialized = 1;
  }

  void VtkFLTKWidget::Enable()
  {
    // if already enabled then done
    if (Enabled) { return; }

    // that's it
    Enabled = 1;
    Modified();
  }

  void VtkFLTKWidget::Disable()
  {
    // if already disabled then done
    if (!Enabled) { return; }

    // that's it (we can't remove the event handler like it should be...)
    Enabled = 0;
    Modified();
  }

  void VtkFLTKWidget::Start()
  {
    // the interactor cannot control the event loop
    vtkErrorMacro(
      << "VtkFLTKWidget::Start() interactor cannot control event loop.");
  }

  void VtkFLTKWidget::SetRenderWindow(vtkRenderWindow* win)
  {
    if (win)
    {
      win->Finalize();
      win->SetMapped(1);

      vtkRenderWindowInteractor::SetRenderWindow(win);
      RenderWindow->SetSize(w(), h());
    }
  }

  vtkRenderWindow* VtkFLTKWidget::GetRenderWindow()
  {
    if (!RenderWindow)
    {
      // create a default vtk window
      this->SetRenderWindow(vtkRenderWindow::New());
    }

    return RenderWindow;
  }

  void VtkFLTKWidget::UpdateSize(int W, int H)
  {
    if (RenderWindow != NULL)
    {
      // if the size changed tell render window
      if ((W != Size[0]) || (H != Size[1]))
      {
        // adjust our (vtkRenderWindowInteractor size)
        Size[0] = W;
        Size[1] = H;
        // and our RenderWindow's size
        RenderWindow->SetSize(W, H);

        // FLTK can move widgets on resize; if that happened, make
        // sure the RenderWindow position agrees with that of the
        // Fl_Gl_Window
        int* pos = RenderWindow->GetPosition();
        if (pos[0] != x() || pos[1] != y())
        {
          RenderWindow->SetPosition(x(), y());
        }
      }
    }
  }

  void OnTimerGlobal(void* p)
  {
    if (p) { ((VtkFLTKWidget*)p)->OnTimer(); }
  }

  int VtkFLTKWidget::CreateTimer(int timertype)
  {
    // to be called every 10 milliseconds, one shot timer
    // we pass "this" so that the correct OnTimer instance will be called
    if (timertype == VTKI_TIMER_FIRST)
    { Fl::add_timeout(0.01, OnTimerGlobal, (void*)this); }
    else
    { Fl::repeat_timeout(0.01, OnTimerGlobal, (void*)this); }

    return 1;
    // Fl::repeat_timer() is more correct, it doesn't measure the timeout
    // from now, but from when the system call that caused this timeout
    // elapsed.
  }
  //---------------------------------------------------------------------------
  int VtkFLTKWidget::DestroyTimer()
  {
    // do nothing
    return 1;
  }

  void VtkFLTKWidget::OnTimer(void)
  {
    if (!Enabled) { return; }
    // this is all we need to do, InteractorStyle is stateful and will
    // continue with whatever it's busy
    this->InvokeEvent(vtkCommand::TimerEvent, NULL);
  }

  void VtkFLTKWidget::TerminateApp() {}

  void VtkFLTKWidget::flush(void) { draw(); }

  void VtkFLTKWidget::draw(void)
  {
    if (RenderWindow)
    {
      assert(m_isReadyForRendering &&
             "makeReadyForFirstRender should be called before any drawing");
      // make sure the vtk part knows where and how large we are
      UpdateSize(this->w(), this->h());

      // make sure the GL context exists and is current:
      // after a hide() and show() sequence e.g. there is no context yet
      // and the Render() will fail due to an invalid context.
      // see Fl_Gl_Window::show()
      make_current();

      // get vtk to render to the Fl_Gl_Window
      Render();
    }
  }

  void VtkFLTKWidget::resize(int lx, int ly, int lw, int lh)
  {
    // make sure VTK knows about the new situation
    UpdateSize(lw, lh);
    // resize the FLTK window by calling ancestor method
    Fl_Gl_Window::resize(lx, ly, lw, lh);
  }

  void VtkFLTKWidget::hide()
  {
    vtkRenderWindow* renderWindow =
      vtkRenderWindowInteractor::GetRenderWindow();
    if (renderWindow)
    {
      // renderWindow->Finalize();
    }
    Fl_Gl_Window::hide();
  }

  void VtkFLTKWidget::show()
  {
    vtkRenderWindow* renderWindow =
      vtkRenderWindowInteractor::GetRenderWindow();
    if (renderWindow)
    {
      // renderWindow->Finalize();
    }
    Fl_Gl_Window::show();
  }

  int VtkFLTKWidget::handle(int event)
  {
    if (!Enabled) { return 0; }

    // setup for new style
    // SEI(x, y, ctrl, shift, keycode, repeatcount, keysym)
    this->SetEventInformation(
      Fl::event_x(), this->h() - Fl::event_y() - 1, Fl::event_state(FL_CTRL),
      Fl::event_state(FL_SHIFT), Fl::event_key(), 1, NULL);

    switch (event)
    {
      case FL_FOCUS:
      case FL_UNFOCUS:;  // Return 1 if you want keyboard events, 0 otherwise.
        // Yes we do
        break;

      case FL_KEYBOARD:  // keypress
        // new style
        this->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
        this->InvokeEvent(vtkCommand::KeyPressEvent, NULL);
        this->InvokeEvent(vtkCommand::CharEvent, NULL);

        // now for possible controversy: there is no way to find out if the
        // InteractorStyle actually did something with this event.  To play
        // it safe (and have working hotkeys), we return "0", which
        // indicates to FLTK that we did NOTHING with this event.  FLTK will
        // send this keyboard event to other children in our group, meaning
        // it should reach any FLTK keyboard callbacks (including hotkeys)
        return 0;
        break;

      case FL_PUSH:            // mouse down
        this->take_focus();  // this allows key events to work
        switch (Fl::event_button())
        {
          case FL_LEFT_MOUSE:

            // new style
            this->InvokeEvent(vtkCommand::LeftButtonPressEvent, NULL);

            break;
          case FL_MIDDLE_MOUSE:

            // new style
            this->InvokeEvent(vtkCommand::MiddleButtonPressEvent, NULL);

            break;
          case FL_RIGHT_MOUSE:

            // new style
            this->InvokeEvent(vtkCommand::RightButtonPressEvent, NULL);

            break;
        }
        break;  // this break should be here, at least according to
      // vtkXRenderWindowInteractor

      // we test for both of these, as fltk classifies mouse moves as with
      // or without button press whereas vtk wants all mouse movement
      // (this bug took a while to find :)
      case FL_DRAG:
      case FL_MOVE:
        // new style
        this->InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
        break;

      case FL_RELEASE:  // mouse up
        switch (Fl::event_button())
        {
          case FL_LEFT_MOUSE:
            // new style
            this->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, NULL);
            break;
          case FL_MIDDLE_MOUSE:
            // new style
            this->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,
                              NULL);
            break;
          case FL_RIGHT_MOUSE:
            // new style
            this->InvokeEvent(vtkCommand::RightButtonReleaseEvent,
                              NULL);
            break;
        }
        break;

      default:  // let the base class handle everything else
        return Fl_Gl_Window::handle(event);

    }  // switch(event)...

    return 1;  // we handled the event if we didn't return earlier
  }

  static char const rcsid[] = "Id";

  const char* VtkFLTKWidget_rcsid(void) { return rcsid; }
}  // namespace tailor::ks