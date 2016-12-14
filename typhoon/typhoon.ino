

enum MenuValueType
{
  INT_TYPE,
  FLOAT_TYPE
};

typedef struct
{
  char text[31];
  MenuValueType menuValueType;
  void * menuSettings;
} Menu_Option;

typedef struct
{
  float maxVal;
  float minVal;
  float (* getCurrentValue)(void);
  void (* saveNewValue)(float);
} FloatMenuSettings;

typedef struct
{
  int maxVal;
  int minVal;
  int (* getCurrentValue)(void);
  void (* saveNewValue)(int);
} IntMenuSettings;


float getDefaultFloat(void)
{
  return 3.456;  
}

int getDefaultInt(void)
{
 return 128; 
}
FloatMenuSettings defaultFloatSettings = {5.123,1.678,&getDefaultFloat};
IntMenuSettings defaultIntMenuSettings = {255,0,&getDefaultInt};
static Menu_Option intMenuOptions [] =
{
 {{"Hello, option 1!"},FLOAT_TYPE,&defaultFloatSettings},
 {{"Another option 2!!"},INT_TYPE,&defaultIntMenuSettings},
 {{"Hello, option 3!"},FLOAT_TYPE,&defaultFloatSettings},
};

static int numIntOptions = sizeof(intMenuOptions) / sizeof(Menu_Option);
static int currentMenuIndex = 0;

void displayMenu(int index)
{
  Serial.println();
  Serial.println(intMenuOptions[index].text);
  FloatMenuSettings floatSettings;
  IntMenuSettings intSettings;
  switch(intMenuOptions[index].menuValueType)
  {
    case FLOAT_TYPE:
      floatSettings = *(FloatMenuSettings *)(intMenuOptions[index].menuSettings);
      Serial.print(floatSettings.getCurrentValue());
      Serial.print(" / ");
      Serial.println(floatSettings.maxVal);
      break;
    case INT_TYPE:
      intSettings = *(IntMenuSettings *)(intMenuOptions[index].menuSettings);
      Serial.print(intSettings.getCurrentValue());
      Serial.print(" / ");
      Serial.println(intSettings.maxVal);
      break;
  }
}

void setup(void) 
{
  Serial.begin(9600); 
  Serial.println("int:");
  Serial.println(sizeof(int));
  Serial.println("long:");
  Serial.println(sizeof(long));
  Serial.println("char:");
  Serial.println(sizeof(char));
  displayMenu(currentMenuIndex);
}

void processMenu(void)
{
  char selection = Serial.read();
  int newMenuIndex = currentMenuIndex;
  switch(selection)
  {
    case 'a':
     newMenuIndex = currentMenuIndex - 1;
     break;
    case 's':
     newMenuIndex = currentMenuIndex + 1;
     break;
  }
  
  if(newMenuIndex != currentMenuIndex)
  {
    if(newMenuIndex < 0)
    {
     newMenuIndex = numIntOptions - 1;
    }
    else if(newMenuIndex == numIntOptions)
    {
      newMenuIndex = 0;
    }
    currentMenuIndex = newMenuIndex;
    displayMenu(currentMenuIndex);
  }
}

void loop(void)
{
  processMenu();
  delay(100);
}
