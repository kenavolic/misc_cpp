
#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

class vtkRenderWindow;
class vtkImageViewer2;

///
/// @brief Custom implementation equivalent to QVtkWidget for FLTK
///
class VtkFLTKWidget : public Fl_Gl_Window, public vtkRenderWindowInteractor
{
public:
  // ctors
  VtkFLTKWidget(int x, int y, int w, int h, const char* l = "");
  // vtk ::New()
  static VtkFLTKWidget* New();
  // dtor
  ~VtkFLTKWidget();

  void makeReadyForFirstRender();
  void resetCamera();
  void checkState() const;

  // vtkRenderWindowInteractor overrides
  void Initialize() override;
  void Enable() override;
  void Disable() override;
  void Start() override;
  void SetRenderWindow(vtkRenderWindow* aren);
  void UpdateSize(int x, int y) override;
  int CreateTimer(int timertype) override;
  int DestroyTimer() override;
  void OnTimer(void);
  void TerminateApp() override;
  vtkRenderWindow* GetRenderWindow();

  void hide() override;
  void show() override;
  void resize(int x, int y, int w, int h) override;

protected:
  // Fl_Gl_Window overrides
  void flush() override;
  void draw() override;
  int handle(int event) override;

private:
  bool m_isReadyForRendering{false};
};

struct VtkFLTKWidgetDeleter
{
  void operator()(VtkFLTKWidget* p) const { p->Delete(); }
};
