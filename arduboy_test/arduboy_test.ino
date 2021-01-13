// text mode: 21x8

#include <Arduboy2.h>

Arduboy2 arduboy;

//volatile long count = 0;
volatile int32_t temp_inter = 0;
volatile uint16_t vcc = 0;

//read internal adc
uint16_t read_adc( uint8_t ref, uint8_t ch)
{
    //uint8_t ch_a = ch & 0b011111;
    //uint8_t ch_b = ch & 0b100000;
    
    //reset adc
    ADMUX  = 0;
    ADCSRB = 0;

    //set voltage reference and channel
    ADMUX |= ref<<6;
    ADMUX |= (ch & 0b011111);
    ADCSRB |= (ch & 0b100000);
    
    _delay_ms(2);
    
    //10-bit resolution with ADC clock speed of 50 kHz to 200 kHz
    //Enable ADC, set prescaller to /64, ADC clock of 16mHz/64=125kHz
    ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    //do a dummy read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);
    
    //actual read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);
    
    return ADC;
}

/*
uint8_t text = 48;
uint8_t count = 0;

void test()
{
  while(count <= 168)
  {
    arduboy.print(char(text));
    text++;
    count++;

    if(count%21 == 0)
    {
        arduboy.print('\n');
    }

    if(text == 128)
    {
        text = 32;
    }
  }
}

void test1()
{
    for(int x = 0; x < 128; x+=4)
    {
      for(int y = 0; y < 64; y+=4)
      {
        //arduboy.drawPixel(x, y, arduboy.generateRandomSeed()%2 );
        uint8_t colour = rand()%2;
        
        for(int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                arduboy.drawPixel(x+i, y+j, colour );
            }
        }
      }
    }
}
*/

// a square player class declaration
class player_t
{
  public:

    int16_t x;  //x coordinate (top left)
    int16_t y;  //y coordinate (top left)
    uint8_t w;  //width
    uint8_t h;  //height
    uint8_t button;
    uint16_t d2;  //the minimal distance^2 for collision

    bool up_movable;
    bool down_movable;
    bool left_movable;
    bool right_movable;

    player_t(){}

    void initial(int16_t x_in, int16_t y_in, uint8_t w_in, uint8_t h_in);
    void draw();
    void coordinate_correct();
    void move_player(uint8_t direc);
    void move_cancel(uint8_t direc);
    uint8_t collision_detect( class object_t &input, uint8_t direc );
    
};


// a square object class declaration
class object_t
{
  public:

    uint8_t id;

    int16_t x;  //x coordinate (top left)
    int16_t y;  //y coordinate (top left)
    uint8_t w;  //width
    uint8_t h;  //height
    uint16_t d2;  //the minimal distance^2 for collision

    uint8_t up_movable;
    uint8_t down_movable;
    uint8_t left_movable;
    uint8_t right_movable;

    object_t(){}

    void initial(uint8_t id_in, int16_t x_in, int16_t y_in, uint8_t w_in, uint8_t h_in);
    void draw();
    void reset_movable();
    void coordinate_correct();
    void border_detect( class object_t &input );
    void display_info();
};


///////////////////////////////////////////////////////
// player class definition/////////////////////////////
void player_t::initial(int16_t x_in, int16_t y_in, uint8_t w_in, uint8_t h_in)
{
    x = x_in;
    y = y_in;
    w = w_in;
    h = h_in;
    d2 = w*w + h*h;

    up_movable = 1;
    down_movable = 1;
    left_movable = 1;
    right_movable = 1;
}

void player_t::draw()
{
    arduboy.drawRect(x, y, w, h, 1);
}

void player_t::coordinate_correct()
{
    if( x < 0) { x = 0;}
    if( y < 0) { y = 0;}

    if( x > WIDTH-w) { x = WIDTH-w; }
    if( y > HEIGHT-h) { y = HEIGHT-h; }
}

void player_t::move_player(uint8_t direc)
{
    button = direc;
    
    switch(direc)
    {
      case 0: // go up
        y--;
        break;
      case 1: // go down
        y++;
        break;
      case 2: // go left
        x--;
        break;
      case 3: // go right
        x++;
        break;
      default:
        break;
    }

    coordinate_correct();
}

void player_t::move_cancel(uint8_t direc)
{
    switch(direc)
    {
      case 0: // go up cancel
        y++;
        break;
      case 1: // go down cancel
        y--;
        break;
      case 2: // go left cancel
        x++;
        break;
      case 3: // go right cancel
        x--;
        break;
      default:
        break;
    }
}

uint8_t player_t::collision_detect( class object_t &input, uint8_t direc )
{
    //collision detected
    if( ( abs(x-input.x) < w )&&( abs(y-input.y) < h ) )
    {
        //check if object can go up
        if( direc == 0)
        {
            //if yes, move the object
            if(input.up_movable == 1)
            {
                input.y--;
            }
            else //if not, cancel the player move
            {
                return 1;
            }
        }

        //check if object can go down
        else if( direc == 1)
        {
            //if yes, move the object
            if(input.down_movable == 1)
            {
                input.y++;
            }
            else //if not, cancel the player move
            {
                return 1;
            }
        }

        //check if object can go left
        else if( direc == 2)
        {
            //if yes, move the object
            if(input.left_movable == 1)
            {
                input.x--;
            }
            else //if not, cancel the player move
            {
                return 1;
            }
        }

        //check if object can go right
        else if( direc == 3)
        {
            //if yes, move the object
            if(input.right_movable == 1)
            {
                input.x++;
            }
            else //if not, cancel the player move
            {
                return 1;
            }
        } 
    }

    return 0;
}

///////////////////////////////////////////////////////
// a square object class definition ///////////////////
void object_t::initial(uint8_t id_in, int16_t x_in, int16_t y_in, uint8_t w_in, uint8_t h_in)
{
    id = id_in;
    x = x_in;
    y = y_in;
    w = w_in;
    h = h_in;
    d2 = w*w + h*h;

    up_movable = 1;
    down_movable = 1;
    left_movable = 1;
    right_movable = 1;
}

void object_t::draw()
{
    arduboy.fillRect(x, y, w, h, 1);
    
    arduboy.drawPixel(x+2, y,   up_movable);    //up
    arduboy.drawPixel(x+2, y+4, down_movable);  //down
    arduboy.drawPixel(x,   y+2, left_movable);  //left
    arduboy.drawPixel(x+4, y+2, right_movable); //right
}

void object_t::coordinate_correct()
{
    if( x == 0 ) { left_movable *= 0; }
    if( y == 0 ) { up_movable *= 0; }
    if( x == WIDTH-w ) { right_movable *= 0; }
    if( y == HEIGHT-h ) { down_movable *= 0; }
}

void object_t::reset_movable()
{
    up_movable = 1;
    down_movable = 1;
    left_movable = 1;
    right_movable = 1;
}

void object_t::border_detect( class object_t &input )
{
    //assume all objects are the same size
    //calculate distance
    uint16_t distance = (input.x-x)*(input.x-x) + (input.y-y)*(input.y-y);

    //too far away, no collison, so skip flag update
    if(distance >= d2) { return; }
    else
    {
        //check up
        if( (y - input.y == h)&&( abs(input.x - x) < w) )
        {
            up_movable *= 0;
            input.down_movable *= 0;
        }
        else
        {
            up_movable *= 1;
            input.down_movable *= 1;
        }
        
        //check down
        if( (input.y - y == h)&&( abs(input.x - x) < w) )
        {
            down_movable *= 0;
            input.up_movable *= 0;
        }
        else
        {
            down_movable *= 1;
            input.up_movable *= 1;
        }

        //check left
        if( (x - input.x == w)&&( abs(input.y - y) < h) )
        {
            left_movable *= 0;
            input.right_movable *= 0;
        }
        else
        {
            left_movable *= 1;
            input.right_movable *= 1;
        }

        //check right
        if( (input.x - x == w)&&( abs(input.y - y) < h) )
        {
            right_movable *= 0;
            input.left_movable *= 0;
        }
        else
        {
            right_movable *= 1;
            input.left_movable *= 1;
        }
    }
}

void object_t::display_info()
{
    Serial.print(id);
    Serial.print(", ");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(", ");
    Serial.print(up_movable);
    Serial.print(", ");
    Serial.print(down_movable);
    Serial.print(", ");
    Serial.print(left_movable);
    Serial.print(", ");
    Serial.println(right_movable);        
}
    

object_t objects[10];
uint8_t object_size = 10;

player_t player;

uint8_t button_direction = 5;
uint8_t flag = 0;



void setup()
{
    Serial.begin(115200);
    
    arduboy.begin();
    //arduboy.setFrameRate(30);
    //arduboy.clear();

    power_adc_enable();

    //test();
    //arduboy.invert(1);

    //arduboy.allPixelsOn(1);
    
    //arduboy.print(char(48));
    //arduboy.display();

    for(int i = 0; i < object_size; i++)
    {
        objects[i].initial(i, 10*i+20, 40, 5, 5);
    }

    player.initial(50, 20, 5, 5);
}


int16_t x_player = 50;
int16_t y_player = 10;

int16_t x_object = 50;
int16_t y_object = 20;

int16_t reset( int16_t axis, uint8_t len)
{
  if(axis < 0) { axis = 0; }
  if(axis > len-5) { axis = len-5; }
  return axis;
}


void loop()
{
    if (!arduboy.nextFrame())
    {  return;  }
    
    arduboy.clear();

    arduboy.setCursor(0, 0);
    vcc = read_adc(0b01, 0b011110 );
    vcc = 1126400/vcc; 
    arduboy.print(vcc);

    arduboy.setCursor(0, 10);
    temp_inter = read_adc(0b11, 0b100111);
    temp_inter = temp_inter - 275;
    arduboy.print(temp_inter);

    //reset all object movable flag
    for(int i = 0; i < object_size; i++)
    {
        objects[i].reset_movable();
        objects[i].coordinate_correct();
    }
    
    //re-calculate object against each other to get movable flag
    for(int i = 0; i < object_size-1; i++)
    {
        for(int j = i+1; j < object_size; j++)
        {
            objects[i].border_detect( objects[j] );
        }
    }
    
    arduboy.pollButtons();

    if(arduboy.justPressed(UP_BUTTON)||arduboy.pressed(UP_BUTTON + A_BUTTON))
    {  button_direction = 0;  }
    
    if(arduboy.justPressed(DOWN_BUTTON)||arduboy.pressed(DOWN_BUTTON + A_BUTTON))
    {  button_direction = 1;  }
    
    if(arduboy.justPressed(LEFT_BUTTON)||arduboy.pressed(LEFT_BUTTON + A_BUTTON))
    {  button_direction = 2;  }
    
    if(arduboy.justPressed(RIGHT_BUTTON)||arduboy.pressed(RIGHT_BUTTON + A_BUTTON))
    {  button_direction = 3;  }

    //pre-move the player
    player.move_player(button_direction);
    
    //check player with all objects
    for(int i = 0; i < object_size; i++)
    {
        flag = player.collision_detect( objects[i], button_direction );
        
        if( flag == 1)
        {
            player.move_cancel(button_direction);
            break;
        }
    }

    button_direction = 5;
    
    //display objects and player
    for(int i = 0; i < object_size; i++)
    {
        objects[i].draw();
    }

    player.draw();

    arduboy.display();

    //objects[0].display_info();
    //while(1){};
}
