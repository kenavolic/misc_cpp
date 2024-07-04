# VtkFLTKWidget

## Description

Prototype for a vtkfltk widget used to display 3D structure in a vtk render
window as part as a GUI build on FLTK

Better projects exist to integrate VTK within an FLTK GUI:

* <https://github.com/pdhahn/vtkFLTK-V8.x-F1.4>

## Usage

Assuming it is contained in `MyFLTKVTKContainerWidget` inheriting from
some `FlWidget`

```cpp

// MyFLTKVTKContainerWidget.h

using UniqueWidgetPtr_T =
        std::unique_ptr<VtkFLTKWidget, VtkFLTKWidgetDeleter>;

// ...

UniqueWidgetPtr_T m_display3DWidget; // MyFLTKVTKContainerWidget member
bool m_isReady{false};

// MyFLTKVTKContainerWidget.cpp

void init() {
  
  // ...
  m_display3DWidget =
          UniqueWidgetPtr_T{new VtkFLTKWidget(x, y, ww3d, hh3d, "Display 3d")};
  // ... (create vtkRenderer)

  // add vtk renderer
  m_display3DWidget->GetRenderWindow()->AddRenderer(vtkRenderer);
  m_display3DWidget->Initialize();

  // add picker, interactor style
  // auto picker = vtkSmartPointer<vtkCellPicker>::New();
  // picker->SetTolerance(0.01);
  // m_display3DWidget->SetPicker(picker);
  // auto style = vtkSmartPointer<_vtkMousePickerInteractorStyle>::New();
  // style->Parent = this;
  // m_display3DWidget->SetInteractorStyle(style);
}

void show() {
  Fl_Widget::show();

  if (m_isReady) {
    return;
  }

  m_display3DWidget->checkState();
  m_display3DWidget->makeReadyForFirstRender();
  // redrawAll();
  m_isReady = true;
}

void update() {
  // call when necessary
  // m_display3DWidget->redraw();
  // m_display3DWidget->resetCamera();
  // m_display3DWidget->resize(0, 0, w(), h());
}
```
