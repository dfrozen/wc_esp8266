#include "HTTP_Server.h"
#include "Settings.h"
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
 
  ESP8266WebServer httpServer(80);
  ESP8266HTTPUpdateServer httpUpdater;

// Create http_server 
HTTP_Server::HTTP_Server(bool inDebugMode)
{
  debugMode = inDebugMode;
  tryingReconnect = false;
}

void HTTP_Server::start(){


  MDNS.begin(update_host);
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTP Update Server ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", update_host, update_path, update_username, update_password);

}
void HTTP_Server::handle(){
    httpServer.handleClient();
    MDNS.update();
}

void HTTP_Server::update(){

}
