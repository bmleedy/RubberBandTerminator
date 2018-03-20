/*
 ESP8266
 
 This class is an ESP8266 for the purposes of my IOT work.  On creation,
 it will set the device to:
  * Only act as a client of an access point, not an access point itself
  * Connect to the house access point
  * Serve a multi-connection TCP server on port 8080
 
 You then interact with it's public methods to use the server:
  * Check_for_requests(bool verbose) - reads one line from the SoftwareSerial
    port and returns TRUE if a line contains a string specified by the caller.

//todo: replace String to reduce heap fragmentation
You #include <string.h> to use those functions. All the names are shorthand, you get to know them.

Some easy basics you can do most simple things with:
strlen() is string length
strcpy() is string copy, it puts the terminating zero at the end of the copy. It is string = string.
strstr() is string string, it searches for a substring within a string
strcmp() string compare tells you if one string is >, ==, or less than another, good for sorting
strcat() string concatenation, adds one string to the end of another

You will some with an n in the middle. The n tells you that character count is used.

strncpy() is strcpy for up to n characters and does NOT put a zero at the end.
strncpy is perfect for writing over part of a string with another, in BASIC it is mid$()

Not simple but very useful is strtok(), string token, that you can use to parse strings with.

Also don't forget the mem (memory) functions, the 2 most basic:
memset(), to set some number of bytes equal to a given value
memmove(), copies bytes and **is safe to use when the destination overlaps the source** 

Measure free memory: https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory 


Put website into flash memory as a string: 

const char *text =
  "This text is pretty long, but will be "
  "concatenated into just a single string. "
  "The disadvantage is that you have to quote "
  "each part, and newlines must be literal as "
  "usual.";

The indentation doesn't matter, since it's not inside the quotes.

You can also do this, as long as you take care to escape the embedded newline. Failure to do so, like my first answer did, will not compile:

const char *text2 =
  "Here, on the other hand, I've gone crazy \
and really let the literal span several lines, \
without bothering with quoting each line's \
content. This works, but you can't indent.";



 
*/
#include "ESP8266.h"



ESP8266::ESP8266(SoftwareSerial *port, bool verbose){
    this->port = port;
    serial_input_buffer.reserve(SERIAL_INPUT_BUFFER_MAX_SIZE);
    this->setup_device();
    this->verbose = verbose;
}

//todo: implement a get_line method to put the next line in the input buffer

bool ESP8266::check_for_request(String matchtext){
    char latest_byte = '\0';
    while (this->port->available()) {
        latest_byte = this->port->read();
        if(verbose){Serial.write(latest_byte);}
        //Need to replace this with a proper ring buffer or long lines could crash me. :-O //todo//
        serial_input_buffer = String(serial_input_buffer + String(latest_byte)); //todo: probably really inefficient
        if(serial_input_buffer.indexOf("\n") != -1) {
            if(serial_input_buffer.indexOf(matchtext) != -1){
                //render_page_for_channel(0,&softPort); //todo deleteme
                this->serial_input_buffer = "";  //todo: leave the input buffer alone instead of clearing it.
                return true;
            }
            else{
                return false;
            }
            
        }
    }
}

bool ESP8266::expect_response_to_command(String command,
                                String response,
                                unsigned int timeout_ms){
    String input_buffer = "";
    input_buffer.reserve(100);
  
    // Write the command
    this->port->write(String(command + "\r\n").c_str());
    if(this->verbose){Serial.println(String(">>>" + command));}
    
    // Spin for timeout_ms
    unsigned int start_time = millis();
    char rv = -1;
    while((millis() - start_time) < timeout_ms){
        
        // Read 1 char off the serial port.
        rv = this->port->read();
        if (rv != -1) {
            if(this->verbose){Serial.write(rv);}
            input_buffer = String(input_buffer + String(rv));
            if(input_buffer.indexOf(response) != -1) {
                return true;
            }
        }//if(rv != 1)
        
    }//while(millis...)
    return false;
}

bool ESP8266::print_response_to_command(String command,
                               unsigned int timeout_ms){
    String input_buffer = "";
    input_buffer.reserve(100);
    
    // Write the command
    this->port->write(String(command + "\r\n").c_str());
    if(this->verbose){Serial.println(String(">>>" + command));}
    
    // Spin for timeout_ms
    unsigned int start_time = millis();
    char rv = -1;
    while((millis() - start_time) < timeout_ms){
        
        // Read 1 char off the serial port.
        rv = this->port->read();
        if (rv != -1) {
            Serial.write(rv);
            input_buffer = String(input_buffer + String(rv));
            if(input_buffer.indexOf("\r\n\r\nOK") != -1) {
                return true;
            }
        }//if(rv != 1)
        
    }//while(millis...)
    return false;
}

bool ESP8266::setup_device(){
    // Get a response from anyone
    Serial.print(F("ESP8266 - Waiting for a response from the Wifi Device..."));
    while(!expect_response_to_command(String("AT"),String("OK"))){
        delay(100);
    }
    Serial.println("[OK]");
    
    Serial.print(F("ESP8266 - Checking the device CWMODE..."));
    // Set myself up as a client of an access point.
    if(expect_response_to_command(String("AT+CWMODE?"),String("+CWMODE:1"))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -    Setting the mode to 'client mode'"));
        if(expect_response_to_command(String("AT+CWMODE=1"),String("OK"))){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Now join the house access point
    Serial.print(F("ESP8266 - Checking that we are on the 'leedy' network..."));
    if(expect_response_to_command(String("AT+CWJAP?"),String("+CWJAP:\"leedy\""))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -     Not on the 'leedy' network.  Changing the WiFi settings to join network..."));
        if(expect_response_to_command(String("AT+CWJAP=\"leedy\",\"teamgoat\""),String("OK"),10000)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Set ourselves up to mux connections into our little server
    Serial.print(F("ESP8266 - Checking the CIPMUX Settings..."));
    if(expect_response_to_command(String("AT+CIPMUX?"),String("+CIPMUX:1"))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -    Server not enabled yet. Setting CIPMUX=1..."));
        if(expect_response_to_command(String("AT+CIPMUX=1"),String("OK"),10000)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Now setup the CIP Server
    Serial.print(F("ESP8266 - Configuring my server on port 8080..."));
    if(expect_response_to_command(String("AT+CIPSERVER=1,8080"),String("OK"),10000)){
        Serial.println(F("[OK]"));
    } else {
        Serial.println(F("[FAIL]"));
        return false;
    }
}


//DEPRECATED: todo: deleteme (too stringy, space-inefficient.)
void ESP8266::send_data(unsigned char channel, String write_data){
    
    // Command the esp to listen for n bytes of data
    String write_command = String(String("AT+CIPSEND=")+//todo: see if I can use the append method
                                  String(channel)+
                                  String(",")+
                                  String(write_data.length())+
                                  String("\r\n"));//todo: figure out if I need to do all the string conversions.
    Serial.println("--------------------------");
    Serial.println(write_data);
    Serial.println("--------------------------");
    Serial.println(write_command);
    Serial.println("--------------------------");
    port->write(write_command.c_str());
    delay(20);
    // Now write the data
    port->write(write_data.c_str());
    delay(20);
    // Now close the connection
    write_command = String(String("AT+CIPCLOSE=")+
                           String(channel)+
                           String("\r\n"));
    // todo: validate that these commands actually worked.  Right now they're open loop.
    port->write(write_command.c_str());
    //https://www.youtube.com/watch?v=ETLDW22zoMA&t=9s
}


//Send the contents of my output queue to a specific channel
void ESP8266::send_output_queue(unsigned char channel){
    
    // Command the esp to listen for n bytes of data  // todo: make less stringy
    String write_command = String(String("AT+CIPSEND=")+//todo: see if I can use the append method
                                  String(channel)+
                                  String(",")+
                                  String(output_queue.get_total_size())+
                                  String("\r\n"));//todo: make this less stringy.  Ok for now, since they're pretty short and all local variables.

    string_element data_to_write;

    //while we can retrieve things from the tou
    while(output_queue.get_element(&data_to_write)){
      port->write(data_to_write.pointer,data_to_write.string_length);
    }
    
    // Now close the connection  // todo: make less stringy
    write_command = String(String("AT+CIPCLOSE=")+
                           String(channel)+
                           String("\r\n"));
    // todo: validate that these commands actually worked.  Right now they're open loop.
    port->write(write_command.c_str());
    //https://www.youtube.com/watch?v=ETLDW22zoMA&t=9s
}



//DEPRECATED todo: deletme (too stringy)
void ESP8266::send_http_200(unsigned char channel, String page_data){

  //todo: delete this method, if I can.
    String content = "HTTP/1.1 200 OK\r\n\r\n";
    
    //todo: maybe don't use string (not sure what happens under there) in favor of a fixed-size char[] response buffer.
    
    //todo: add Content_Length header so I don't have to close the connection. Or, maybe I want to close the connection anyway.
    // https://www.w3.org/Protocols/HTTP/Response.html
    
    Serial.println("--------------------------");
    Serial.println(content);
    
    content = String(content + page_data); //content is not propagating here - out of memory //bml - left off here.
    
    Serial.println("--------------------------");
    Serial.println(content);
    this->send_data(channel, content);
}

void ESP8266::send_http_200_static(unsigned char channel, char page_data[], unsigned int page_data_len){

    int total_page_size = 0;

    // Save the HTTP header from PROGMEM into the output buffer
    const char header[] PROGMEM = "HTTP/1.1 200 OK\r\n\r\n";
    this->output_queue.add_element(header,19);
    //todo: add Content_Length header so I don't have to close the connection. Or, maybe I want to close the connection anyway.
    // https://www.w3.org/Protocols/HTTP/Response.html

    // Now enqueue the website page data
    this->output_queue.add_element(page_data,page_data_len);
    
    // Send!
    this->send_output_queue(channel);
}

OutputQueue::OutputQueue(){
  this->clear_elements();
}

// Add an element to this queue.
void OutputQueue::add_element(char * string, unsigned char string_len){
  if(queue_len >= MAX_OUTPUT_QUEUE_LENGTH){
    Serial.println(F("| OutputQueue::add_element: Max output queue length exceeded! Check your code!"));
  }else if(read_position != 0){
    Serial.println(F("| OutputQueue::add_element: Cannot add elements to an output queue that's partially read!"));
  }else{
    queue[queue_len].pointer = string;
    queue[queue_len].string_length = string_len;
    total_size += string_len;  //tally up size of referenced strings
    queue_len++;
  }
}

// Clear the queue and the read position at the same time
void OutputQueue::clear_elements(){
  //no need to actually erase the data, just reset list position
  queue_len = 0;
  read_position = 0;
  total_size = 0;
}

//returns false if none are available.
//resets the queue and returns false when there is nothing left to read
bool OutputQueue::get_element(string_element * output){
  if(read_position >= queue_len){
    //nothing to read
    this->clear_elements();
    return false;
  } else {
    *output = queue[read_position];
  }
}

unsigned int OutputQueue::get_total_size(){
  return this->total_size;
}

