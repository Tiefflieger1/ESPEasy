#ifdef USES_P001
//#######################################################################################################
//#################################### Plugin 001: Input Switch #########################################
//#######################################################################################################

/**************************************************\
CONFIG
TaskDevicePluginConfig settings:
0: button type (switch or dimmer)
1: dim value
2: button option (normal, push high, push low)
3: send boot state (true,false)
4: use doubleclick (0,1,2,3)
5: use longpress (0,1,2,3)
6: LP fired (true,false)
7: doubleclick counter (=0,1,2,3)

TaskDevicePluginConfigFloat settings:
0: debounce interval ms
1: doubleclick interval ms
2: longpress interval ms
3: use safebutton (=0,1)

TaskDevicePluginConfigLong settings:
0: clickTime debounce ms
1: clickTime doubleclick ms
2: clickTime longpress ms
3: safebutton counter (=0,1)
\**************************************************/

#define PLUGIN_001
#define PLUGIN_ID_001         1
#define PLUGIN_NAME_001       "Switch input - Switch"
#define PLUGIN_VALUENAME1_001 "Switch"
// Make sure the initial default is a switch (value 0)
#define PLUGIN_001_TYPE_SWITCH 0
#define PLUGIN_001_TYPE_DIMMER 3 // Due to some changes in previous versions, do not use 2.
#define PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH 0
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW 1
#define PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH 2
#define PLUGIN_001_DOUBLECLICK_MIN_INTERVAL 1000
#define PLUGIN_001_DOUBLECLICK_MAX_INTERVAL 3000
#define PLUGIN_001_LONGPRESS_MIN_INTERVAL 1000
#define PLUGIN_001_LONGPRESS_MAX_INTERVAL 5000
#define PLUGIN_001_DC_DISABLED 0
#define PLUGIN_001_DC_LOW 1
#define PLUGIN_001_DC_HIGH 2
#define PLUGIN_001_DC_BOTH 3
#define PLUGIN_001_LONGPRESS_DISABLED 0
#define PLUGIN_001_LONGPRESS_LOW 1
#define PLUGIN_001_LONGPRESS_HIGH 2
#define PLUGIN_001_LONGPRESS_BOTH 3


// Needed for the GPIO handling.
#include "Commands/Gpio.h"
// As soon as we allow switch plugin to be used on port expander, we may need to move those functions.
// For now, the Switch plugin only supports the internal GPIO pins.

boolean Plugin_001(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;
  //static byte switchstate[TASKS_MAX];
  //static byte outputstate[TASKS_MAX];
  //static int8_t PinMonitor[GPIO_MAX];
  //static int8_t PinMonitorState[GPIO_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_001;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].InverseLogicOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_001);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_001));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        // FIXME TD-er: This plugin is handling too much.
        // - switch/dimmer input
        // - PWM output
        // - switch output (relays)
        // - servo output
        // - sending pulses
        // - playing tunes
        event->String1 = formatGpioName_bidirectional("");
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //@giig1967g: set current task value for taking actions after changes in the task gpio
        const uint32_t key = createInternalGpioKey(Settings.TaskDevicePin1[event->TaskIndex]);
        if (existPortStatus(key)) {
          globalMapPortStatus[key].previousTask = event->TaskIndex;
        }

        String options[2];
        options[0] = F("Switch");
        options[1] = F("Dimmer");
        int optionValues[2] = { PLUGIN_001_TYPE_SWITCH, PLUGIN_001_TYPE_DIMMER };
        const byte switchtype = P001_getSwitchType(event);
        addFormSelector(F("Switch Type"), F("p001_type"), 2, options, optionValues, switchtype);

        if (switchtype == PLUGIN_001_TYPE_DIMMER)
        {
          addFormNumericBox(F("Dim value"), F("p001_dimvalue"), Settings.TaskDevicePluginConfig[event->TaskIndex][1], 0, 255);
        }

        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        String buttonOptions[3];
        buttonOptions[0] = F("Normal Switch");
        buttonOptions[1] = F("Push Button Active Low");
        buttonOptions[2] = F("Push Button Active High");
        int buttonOptionValues[3] = {PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW, PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH};
        addFormSelector(F("Switch Button Type"), F("p001_button"), 3, buttonOptions, buttonOptionValues, choice);

        addFormCheckBox(F("Send Boot state"),F("p001_boot"),
        		Settings.TaskDevicePluginConfig[event->TaskIndex][3]);

        addFormSubHeader(F("Advanced event management"));

        addFormNumericBox(F("De-bounce (ms)"), F("p001_debounce"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0]), 0, 250);

        //set minimum value for doubleclick MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] < PLUGIN_001_DOUBLECLICK_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = PLUGIN_001_DOUBLECLICK_MIN_INTERVAL;

        byte choiceDC = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        String buttonDC[4];
        buttonDC[0] = F("Disabled");
        buttonDC[1] = F("Active only on LOW (EVENT=3)");
        buttonDC[2] = F("Active only on HIGH (EVENT=3)");
        buttonDC[3] = F("Active on LOW & HIGH (EVENT=3)");
        int buttonDCValues[4] = {PLUGIN_001_DC_DISABLED, PLUGIN_001_DC_LOW, PLUGIN_001_DC_HIGH,PLUGIN_001_DC_BOTH};

        addFormSelector(F("Doubleclick event"), F("p001_dc"), 4, buttonDC, buttonDCValues, choiceDC);

        addFormNumericBox(F("Doubleclick max. interval (ms)"), F("p001_dcmaxinterval"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1]), PLUGIN_001_DOUBLECLICK_MIN_INTERVAL, PLUGIN_001_DOUBLECLICK_MAX_INTERVAL);

        //set minimum value for longpress MIN max speed
        if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] < PLUGIN_001_LONGPRESS_MIN_INTERVAL)
          Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = PLUGIN_001_LONGPRESS_MIN_INTERVAL;

        byte choiceLP = Settings.TaskDevicePluginConfig[event->TaskIndex][5];
        String buttonLP[4];
        buttonLP[0] = F("Disabled");
        buttonLP[1] = F("Active only on LOW (EVENT= 10 [NORMAL] or 11 [INVERSED])");
        buttonLP[2] = F("Active only on HIGH (EVENT= 11 [NORMAL] or 10 [INVERSED])");
        buttonLP[3] = F("Active on LOW & HIGH (EVENT= 10 or 11)");
        int buttonLPValues[4] = {PLUGIN_001_LONGPRESS_DISABLED, PLUGIN_001_LONGPRESS_LOW, PLUGIN_001_LONGPRESS_HIGH,PLUGIN_001_LONGPRESS_BOTH};
        addFormSelector(F("Longpress event"), F("p001_lp"), 4, buttonLP, buttonLPValues, choiceLP);

        addFormNumericBox(F("Longpress min. interval (ms)"), F("p001_lpmininterval"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2]), PLUGIN_001_LONGPRESS_MIN_INTERVAL, PLUGIN_001_LONGPRESS_MAX_INTERVAL);

        addFormCheckBox(F("Use Safe Button (slower)"), F("p001_sb"), round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3]));

        //TO-DO: add Extra-Long Press event
        //addFormCheckBox(F("Extra-Longpress event (20 & 21)"), F("p001_elp"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]);
        //addFormNumericBox(F("Extra-Longpress min. interval (ms)"), F("p001_elpmininterval"), Settings.TaskDevicePluginConfigLong[event->TaskIndex][2], 500, 2000);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p001_type"));
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == PLUGIN_001_TYPE_DIMMER)
        {
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p001_dimvalue"));
        }

        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("p001_button"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = isFormItemChecked(F("p001_boot"));

        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0] = getFormItemInt(F("p001_debounce"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("p001_dc"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = getFormItemInt(F("p001_dcmaxinterval"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("p001_lp"));
        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = getFormItemInt(F("p001_lpmininterval"));

        Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3] = isFormItemChecked(F("p001_sb"));

        //TO-DO: add Extra-Long Press event
        //Settings.TaskDevicePluginConfigLong[event->TaskIndex][1] = isFormItemChecked(F("p001_elp"));
        //Settings.TaskDevicePluginConfigLong[event->TaskIndex][2] = getFormItemInt(F("p001_elpmininterval"));

        //check if a task has been edited and remove 'task' bit from the previous pin
        for (std::map<uint32_t,portStatusStruct>::iterator it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it) {
          if (it->second.previousTask == event->TaskIndex && getPluginFromKey(it->first)==PLUGIN_ID_001) {
            globalMapPortStatus[it->first].previousTask = -1;
            removeTaskFromPort(it->first);
            break;
          }
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        //apply INIT only if PORT is in range. Do not start INIT if port not set in the device page.
        if (Settings.TaskDevicePin1[event->TaskIndex] >= 0 && Settings.TaskDevicePin1[event->TaskIndex] <= PIN_D_MAX)
        {
          portStatusStruct newStatus;
          const uint32_t key = createInternalGpioKey(Settings.TaskDevicePin1[event->TaskIndex]);
          //Read current status or create empty if it does not exist
          newStatus = globalMapPortStatus[key];

          // read and store current state to prevent switching at boot time
          newStatus.state = read_GPIO_state(event);
          newStatus.output = newStatus.state;
          newStatus.task++; // add this GPIO/port as a task

          //setPinState(PLUGIN_ID_001, Settings.TaskDevicePin1[event->TaskIndex], PIN_MODE_INPUT, switchstate[event->TaskIndex]);
          //  if it is in the device list we assume it's an input pin
          if (Settings.TaskDevicePin1PullUp[event->TaskIndex]) {
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
            newStatus.mode = PIN_MODE_INPUT_PULLUP;
          } else {
            pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT);
            newStatus.mode = PIN_MODE_INPUT;
          }
          // if boot state must be send, inverse default state
          // this is done to force the trigger in PLUGIN_TEN_PER_SECOND
          if (Settings.TaskDevicePluginConfig[event->TaskIndex][3])
          {
            newStatus.state = !newStatus.state;
            newStatus.output = !newStatus.output;
          }

          // set initial UserVar of the switch
          if (Settings.TaskDevicePin1Inversed[event->TaskIndex]){
            UserVar[event->BaseVarIndex] = !newStatus.state;
          } else {
            UserVar[event->BaseVarIndex] = newStatus.state;
          }

          // counters = 0
          Settings.TaskDevicePluginConfig[event->TaskIndex][7]=0;     //doubleclick counter
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][3]=0; //safebutton counter

          //used to track if LP has fired
          Settings.TaskDevicePluginConfig[event->TaskIndex][6]=false;

          //store millis for debounce, doubleclick and long press
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]=millis(); //debounce timer
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]=millis(); //doubleclick timer
          Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]=millis(); //longpress timer

          //set minimum value for doubleclick MIN interval speed
          if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] < PLUGIN_001_DOUBLECLICK_MIN_INTERVAL)
            Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1] = PLUGIN_001_DOUBLECLICK_MIN_INTERVAL;

          //set minimum value for longpress MIN interval speed
          if (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] < PLUGIN_001_LONGPRESS_MIN_INTERVAL)
            Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2] = PLUGIN_001_LONGPRESS_MIN_INTERVAL;

          savePortStatus(key,newStatus);
        }
        success = true;
        break;
      }

    case PLUGIN_REQUEST:
      {
        //String device = parseString(string, 1);
        //String command = parseString(string, 2);
        //String strPar1 = parseString(string, 3);

        // returns pin value using syntax: [plugin#gpio#pinstate#xx]
        if (string.length()>=13 && string.substring(0,13).equalsIgnoreCase(F("gpio,pinstate")))
        {
          int par1;
            if (validIntFromString(parseString(string, 3), par1)) {
            string = digitalRead(par1);
          }
          success = true;
        }
        break;
      }

    case PLUGIN_UNCONDITIONAL_POLL:
      {
        // port monitoring, generates an event by rule command 'monitor,gpio,port#'
        for (std::map<uint32_t,portStatusStruct>::iterator it=globalMapPortStatus.begin(); it!=globalMapPortStatus.end(); ++it) {
          if ((it->second.monitor || it->second.command || it->second.init) && getPluginFromKey(it->first)==PLUGIN_ID_001) {
            const uint16_t port = getPortFromKey(it->first);
            byte state = read_GPIO_state(port, it->second.mode);
            if (it->second.state != state) {
              if (!it->second.task) it->second.state = state; //do not update state if task flag=1 otherwise it will not be picked up by 10xSEC function
              if (it->second.monitor) {
                String eventString = F("GPIO#");
                eventString += port;
                eventString += '=';
                eventString += state;
                rulesProcessing(eventString);
              }
            }
          }
        }
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        const boolean state = read_GPIO_state(event);

        /**************************************************************************\
        20181009 - @giig1967g: new doubleclick logic is:
        if there is a 'state' change, check debounce period.
        Then if doubleclick interval exceeded, reset Settings.TaskDevicePluginConfig[event->TaskIndex][7] to 0
        Settings.TaskDevicePluginConfig[event->TaskIndex][7] contains the current status for doubleclick:
        0: start counting
        1: 1st click
        2: 2nd click
        3: 3rd click = doubleclick event if inside interval (calculated as: '3rd click time' minus '1st click time')

        Returned EVENT value is = 3 always for doubleclick
        In rules this can be checked:
        on Button#Switch=3 do //will fire if doubleclick
        \**************************************************************************/

        //long difftimer1 = 0;
        //long difftimer2 = 0;
        //long timerstats = millis();

        //Bug fixed: avoid 10xSEC in case of a non-fully configured device (no GPIO defined yet)
        if (Settings.TaskDevicePin1[event->TaskIndex]>=0 && Settings.TaskDevicePin1[event->TaskIndex]<=PIN_D_MAX) {

          portStatusStruct currentStatus;
          const uint32_t key = createInternalGpioKey(Settings.TaskDevicePin1[event->TaskIndex]);
          //WARNING operator [],creates an entry in map if key doesn't exist:
          currentStatus = globalMapPortStatus[key];

          //CASE 1: using SafeButton, so wait 1 more 100ms cycle to acknowledge the status change
          //QUESTION: MAYBE IT'S BETTER TO WAIT 2 CYCLES??
          if (round(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][3]) && state != currentStatus.state && Settings.TaskDevicePluginConfigLong[event->TaskIndex][3]==0)
          {
            addLog(LOG_LEVEL_DEBUG,"SW  :SafeButton activated")
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = 1;
          }
          //CASE 2: not using SafeButton, or already waited 1 more 100ms cycle, so proceed.
          else if (state != currentStatus.state)
          {
            // Reset SafeButton counter
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = 0;

            //reset timer for long press
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]=millis();
            Settings.TaskDevicePluginConfig[event->TaskIndex][6] = false;

            const unsigned long debounceTime = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][0]);
            if (debounceTime >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][0])) //de-bounce check
            {
              const unsigned long deltaDC = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]);
              if ((deltaDC >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][1])) ||
                   Settings.TaskDevicePluginConfig[event->TaskIndex][7]==3)
              {
                //reset timer for doubleclick
                Settings.TaskDevicePluginConfig[event->TaskIndex][7]=0;
                Settings.TaskDevicePluginConfigLong[event->TaskIndex][1]=millis();
              }

  //just to simplify the reading of the code
  #define COUNTER Settings.TaskDevicePluginConfig[event->TaskIndex][7]
  #define DC Settings.TaskDevicePluginConfig[event->TaskIndex][4]

                //check settings for doubleclick according to the settings
                if ( COUNTER!=0 || ( COUNTER==0 && (DC==3 || (DC==1 && state==0) || (DC==2 && state==1))) )
                  Settings.TaskDevicePluginConfig[event->TaskIndex][7]++;
  #undef DC
  #undef COUNTER

              currentStatus.state = state;
              const boolean currentOutputState = currentStatus.output;
              boolean new_outputState = currentOutputState;
              switch(Settings.TaskDevicePluginConfig[event->TaskIndex][2])
              {
                case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                    new_outputState = state;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:
                  if (!state)
                    new_outputState = !currentOutputState;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:
                  if (state)
                    new_outputState = !currentOutputState;
                  break;
              }

              // send if output needs to be changed
              if (currentOutputState != new_outputState)
              {
                byte output_value;
                currentStatus.output = new_outputState;
                boolean sendState = new_outputState;

                if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
                  sendState = !sendState;

                if (Settings.TaskDevicePluginConfig[event->TaskIndex][7]==3 && Settings.TaskDevicePluginConfig[event->TaskIndex][4]>0)
                {
                  output_value = 3; //double click
                } else {
                  output_value = sendState ? 1 : 0; //single click
                }
                event->sensorType = SENSOR_TYPE_SWITCH;
                if (P001_getSwitchType(event) == PLUGIN_001_TYPE_DIMMER) {
                  if (sendState) {
                    output_value = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
                    // Only set type to being dimmer when setting a value else it is "switched off".
                    event->sensorType = SENSOR_TYPE_DIMMER;
                  }
                }
                UserVar[event->BaseVarIndex] = output_value;
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("SW  : GPIO=");
                  log += Settings.TaskDevicePin1[event->TaskIndex];
                  log += F(" State=");
                  log += state ? '1' : '0';
                  log += output_value==3 ? F(" Doubleclick=") : F(" Output value=");
                  log += output_value;
                  addLog(LOG_LEVEL_INFO, log);
                }
                sendData(event);

                //reset Userdata so it displays the correct state value in the web page
                UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
              }
              Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = millis();
            }
            savePortStatus(key,currentStatus);
          }

  //just to simplify the reading of the code
  #define LP Settings.TaskDevicePluginConfig[event->TaskIndex][5]
  #define FIRED Settings.TaskDevicePluginConfig[event->TaskIndex][6]

          //CASE 3: status unchanged. Checking longpress:
          //Check if LP is enabled and if LP has not fired yet
          else if (!FIRED && (LP==3 ||(LP==1 && state==0)||(LP==2 && state==1) ) ) {

  #undef LP
  #undef FIRED

            /**************************************************************************\
            20181009 - @giig1967g: new longpress logic is:
            if there is no 'state' change, check if longpress interval reached
            When reached send longpress event.
            Returned Event value = state + 10
            So if state = 0 => EVENT longpress = 10
            if state = 1 => EVENT longpress = 11
            So we can trigger longpress for high or low contact

            In rules this can be checked:
            on Button#Switch=10 do //will fire if longpress when state = 0
            on Button#Switch=11 do //will fire if longpress when state = 1
            \**************************************************************************/
            // Reset SafeButton counter
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = 0;

            const unsigned long deltaLP = timePassedSince(Settings.TaskDevicePluginConfigLong[event->TaskIndex][2]);
            if (deltaLP >= (unsigned long)lround(Settings.TaskDevicePluginConfigFloat[event->TaskIndex][2]))
            {
              byte output_value;
              byte needToSendEvent = false;

              Settings.TaskDevicePluginConfig[event->TaskIndex][6] = true;

              switch(Settings.TaskDevicePluginConfig[event->TaskIndex][2])
              {
                case PLUGIN_001_BUTTON_TYPE_NORMAL_SWITCH:
                    needToSendEvent = true;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_LOW:
                  if (!state)
                    needToSendEvent = true;
                  break;
                case PLUGIN_001_BUTTON_TYPE_PUSH_ACTIVE_HIGH:
                  if (state)
                    needToSendEvent = true;
                  break;
              }

              if (needToSendEvent) {
                boolean sendState = state;
                if (Settings.TaskDevicePin1Inversed[event->TaskIndex])
                  sendState = !sendState;
                output_value = sendState ? 11 : 10;
                //output_value = output_value + 10;

                UserVar[event->BaseVarIndex] = output_value;
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("SW  : LongPress: GPIO= ");
                  log += Settings.TaskDevicePin1[event->TaskIndex];
                  log += F(" State=");
                  log += state ? '1' : '0';
                  log += F(" Output value=");
                  log += output_value;
                  addLog(LOG_LEVEL_INFO, log);
                }
                sendData(event);

                //reset Userdata so it displays the correct state value in the web page
                UserVar[event->BaseVarIndex] = sendState ? 1 : 0;
              }
              savePortStatus(key,currentStatus);
            }
          } else {
            // Reset SafeButton counter
            Settings.TaskDevicePluginConfigLong[event->TaskIndex][3] = 0;
          }
        }
        success = true;
        break;
      }

    case PLUGIN_EXIT:
    {
      removeTaskFromPort(createInternalGpioKey(Settings.TaskDevicePin1[event->TaskIndex]));
      break;
    }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("SW   : State ");
          log += UserVar[event->BaseVarIndex];
          addLog(LOG_LEVEL_INFO, log);
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        String command = parseString(string, 1);
        if (command == F("inputswitchstate")) {
          if (checkValidGpioPin(event->Par1)) {
            portStatusStruct tempStatus;
            const uint32_t   key = createInternalGpioKey(Settings.TaskDevicePin1[event->Par1]);

            // WARNING: operator [] creates an entry in the map if key does not exist
            // So the next command should be part of each command:
            tempStatus = globalMapPortStatus[key];

            UserVar[event->Par1 * VARS_PER_TASK] = event->Par2;
            tempStatus.output                    = event->Par2;
            tempStatus.command                   = 1;
            savePortStatus(key, tempStatus);
            success = true;
          }
        }
        break;
      }

    case PLUGIN_TIMER_IN:
      {
        digitalWrite(event->Par1, event->Par2);
        //setPinState(PLUGIN_ID_001, event->Par1, PIN_MODE_OUTPUT, event->Par2);
        portStatusStruct tempStatus;
        // WARNING: operator [] creates an entry in the map if key does not exist
        const uint32_t key = createInternalGpioKey(event->Par1);
        tempStatus = globalMapPortStatus[key];

        tempStatus.state = event->Par2;
        tempStatus.mode = PIN_MODE_OUTPUT;
        savePortStatus(key,tempStatus);
        break;
      }
  }
  return success;
}


// TD-er: Needed to fix a mistake in earlier fixes.
byte P001_getSwitchType(struct EventStruct *event) {
  byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
  switch (choice) {
    case 2: // Old implementation for Dimmer
    case PLUGIN_001_TYPE_DIMMER:
      choice = PLUGIN_001_TYPE_DIMMER;
      break;
    case 1: // Old implementation for switch
    case PLUGIN_001_TYPE_SWITCH:
    default:
      choice = PLUGIN_001_TYPE_SWITCH;
      break;
  }
  return choice;
}

#endif // USES_P001
