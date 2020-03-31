#pragma once 
extern bool hasWIFI;

class HTTP_Server
{
  private:
    bool debugMode;
    bool tryingReconnect;

  public:
    HTTP_Server(bool inDebugMode = false);
    void start();
    void handle();
    void update();
};
