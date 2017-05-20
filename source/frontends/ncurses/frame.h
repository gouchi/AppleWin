#pragma once

#include <memory>

#include <ncurses.h>

class Frame
{
 public:
  Frame();

  WINDOW * getWindow();
  WINDOW * getBuffer();

  void init(int columns);
  int getColumns() const;

 private:

  int myColumns;

  std::shared_ptr<WINDOW> myFrame;
  std::shared_ptr<WINDOW> myBuffer;
  std::shared_ptr<WINDOW> myBorders;

};