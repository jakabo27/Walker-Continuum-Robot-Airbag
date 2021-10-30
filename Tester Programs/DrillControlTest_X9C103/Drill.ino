void DrillSwitchOFF()
{
  aw.digitalWrite(DRILL_SW_PIN, 1);
}

void DrillSwitchON()
{
  aw.digitalWrite(DRILL_SW_PIN, 0);
}

void DrillClockwise()
{

  if(aw.digitalRead(DRILL_DIR_PIN) != 1)
  {
    //Need to really tell the drill we want to change directions
    aw.digitalWrite(DRILL_DIR_PIN, 1); 
    delay(50);
    DrillSwitchON();
    aw.digitalWrite(DRILL_DIR_PIN, 1); 
    delay(30);
    DrillSwitchOFF();
    aw.digitalWrite(DRILL_DIR_PIN, 1); 
  }
  else
  {
    aw.digitalWrite(DRILL_DIR_PIN, 1); 
  }
  
}

void DrillCCW()
{

  bool needSwitchFlutter = false;
  if(aw.digitalRead(DRILL_DIR_PIN) != 0)
  {
    //Need to really tell the drill we want to change directions
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
    delay(70);
    DrillSwitchON();
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
    delay(30);
    DrillSwitchOFF();
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
    //delay(100);
  }
  else
  {
    aw.digitalWrite(DRILL_DIR_PIN, 0); 
  }
}
