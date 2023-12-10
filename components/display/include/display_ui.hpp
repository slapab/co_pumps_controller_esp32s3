#pragma once

class DisplayUI
{
 public:
   static DisplayUI& instance();

   void init();
   void tick();

 protected:
   /**
    * @brief Some additional initialization. e.g. widgets label or sth like this
    */
   void setup_ui();
};
