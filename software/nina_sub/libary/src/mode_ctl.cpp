#include "mode_ctl.hpp"
#include <string>
#include <iostream>

/*
TODO:
Testzwecke master ctl via ROS
zu ueberlegen, auto master zu slave bzw slave zu master um schneller mehr daten zu bekommen
*/



fpga_mode::fpga_mode(hardware_interface *hw_, msg_gen *trans_,esp_mqtt_client_handle_t *mqttclient_){ //hw *hw_){
  //mqttclient = mqttclient_;
  trans = trans_;
  hw = hw_;

  id = hw->getID();//(uint8_t)(IORD(base_addr_, (uint32_t)(0x00<<8|0)));
  mode_pub = id;
  mode = mode_pub + 1; //has to be different to init as new mode*/

  pub_cnt = 0;


  std::stringstream ss_buffer;

  ss_buffer << "/triangulation/" << +(int)id << "/time_data/";
  time_frame_topic = ss_buffer.str();
  ss_buffer.str("");

}

void fpga_mode::transmission_init(esp_mqtt_client_handle_t *mqttclient_){
  std::stringstream ss_buffer;
  //struct msg_mqtt msg_mqtt;
    /*

  struct simple_msg{
    struct mqtt_out{
      bool allow_input;
      float trigger_time[MAX_LIST_SIZE];
      uint8_t master_identifier[MAX_LIST_SIZE];
    }mqtt_out;
    struct mqtt_in{
      std::vector<uint8_t> masterlist;
      bool     start_burst;
      bool     start_continiouse;
      bool     start_ptp_sync;
      uint32_t burst_cycles;
    }mqtt_in;
  };


  */
  register_sub(mqttclient_,"/triangulation/master/masterlist/", &fpga_mode::get_masterlist);
  register_sub(mqttclient_,"/triangulation/master/allow_input/", &fpga_mode::allow_input);
  register_sub(mqttclient_,"/triangulation/master/start_continiouse/", &fpga_mode::start_continiouse);
  register_sub(mqttclient_,"/triangulation/master/start_ptp_sync/", &fpga_mode::start_ptp_sync);
  register_sub(mqttclient_,"/triangulation/master/burst_cycles/", &fpga_mode::burst_cycles);
  register_sub(mqttclient_,"/triangulation/master/start_burst/", &fpga_mode::start_burst);
  register_sub(mqttclient_,"/triangulation/master/start_conv/", &fpga_mode::start_conv_mqtt);
  ss_buffer.str("");
}

/*
void fpga_mode::transmission_init(){
  esp_mqtt_client_register_event(*mqttclient, MQTT_EVENT_DATA, foo, *mqttclient);
  (void)esp_mqtt_client_subscribe(*mqttclient, "/topic/qos3", 0);
}*/

void fpga_mode::select_subf(uint32_t location, string data_){
  //printf("\nsub subf: %d\n", location);
  (*this.*sub_func_list[location])(data_);       //<--- her the overflow probably happens ...
}

void fpga_mode::register_sub(esp_mqtt_client_handle_t *mqttclient_,string topic_,funcPtr func_){
  topic_list_sub[pub_cnt] = topic_;//"/topic/qos3";
  sub_func_list[pub_cnt] = func_;//&fpga_mode::get_slav_time;
  //const char *topic_buff = topic_.c_str();

  cout << "\n reg sub " << topic_;
  (void)esp_mqtt_client_subscribe(*mqttclient_, topic_.c_str(), 0);


  //trans->creat_topics();
  pub_cnt++;
}
//--------------------------------SUB - FUNCTIONS -----------
void fpga_mode::start_conv_mqtt(string data_){
  //printf("\nCONVERSATION\n");
  uint32_t i;
  istringstream(data_) >> i;

  current_master = i;
  enable_input = false;

  if(i == id){
    mode = MODE_MASTER;
    fp_start_conv = &fpga_mode::master_init;  //default &fpga_mode::slave_init
  }else{
    enable_input=true;
    mode = MODE_SLAVE;
    fp_start_conv = &fpga_mode::slave_init;  //default &fpga_mode::slave_init
  }
  ((*this).*(fp_start_conv))();
  conversation();

}
//static bool get_masterlist_xBIT = 0;
void fpga_mode::get_masterlist(string data_){ // Format "0,1,2,3,4;"
  //while(get_masterlist_xBIT != 0);
  //get_masterlist_xBIT = 1;
  printf("\n-==MASTERLIST===\n");
  //cout << "original data: " << data_ << "\n";
  const char *data_buffer = data_.c_str();
  uint16_t digit_buffer = 0;

  bool err_dat = false;

  for(uint8_t i = 0; i < 100; i++){
    if(*data_buffer != ',' && *data_buffer != ';'){
      if((uint8_t)*data_buffer > 57 || (uint8_t)*data_buffer < 48)
        break;
      digit_buffer += (uint16_t)*data_buffer - 48;
      digit_buffer *=10;
      //printf("\ncnt:  %d", i);
      //printf("\nSTRING DATA:  %c", *data_buffer);
      //printf("\nconv DATA:  %d", digit_buffer);
      //printf("\nraw conv DATA: %d", (uint8_t)*data_buffer);

      data_buffer++;
    }
    else{
      if(i == 0){
        cout << "\n[ERROR] MASTERLIST EMPTY";
        return;
      }
      //cout<<"\nMaster ID: " << digit_buffer;
      digit_buffer = digit_buffer/10;
      push_vec(masterlist, digit_buffer);
      //*list++ = digit_buffer;
      digit_buffer = 0;
      if(*data_buffer == ';')
        break;

      data_buffer++;
    }
  }

  //cout << "\n==END MASTERLIST ==\n";
  if(masterlist.size() == 0){
    for(uint8_t i = 0; i < 7; i++){
      //masterlist.resize(0)
      push_vec(masterlist, 0);
      push_vec(masterlist, 1);
      push_vec(masterlist, 2);
    }
  }
  cout << "\nlist" << masterlist.size();
  for(uint8_t i = 0; i < masterlist.size(); i++){
    //printf("\nList: (%d):  ", i);
    printf("\nList: (%d):  %d", i, masterlist[i]);
    //masterlist.pop_back();

  }
  /*for(uint8_t i = 0; masterlist.size() == 0; i++){
    //printf("\nList: (%d):  ", i);
    printf("\nList: (%d):  %d", i, masterlist.front());
    //printf("\n...List: (%d):  %d", i, masterlist.back());
    pop_vec(masterlist);
    //masterlist.pop_back();

  }*/

  //cout <<"\nList: (0)" << masterlist.at(0) << "..(1)" << masterlist.at(1);

  //std::vector<int> master_list;
//get_masterlist_xBIT = 0;
}
void fpga_mode::start_burst(string data_){
  printf("\n-==start_burst===\n");
  hw->piezo_burst_out();

}
void fpga_mode::start_continiouse(string data_){
  printf("\n-==start_continiouse===\n");
  uint32_t i;
  istringstream(data_) >> i;

  printf("contin: %d\n", i);

  if(i == 0)
    hw->stop_US_out();
  else {
    cout << "\nSTART US\n";
    hw->start_US_out();
  }

}
void fpga_mode::start_ptp_sync(string data_){
  printf("\n-==start_ptp_sync===\n");

  //send_time_frame(0.2);
}
void fpga_mode::burst_cycles(string data_){
  printf("\n-==burst_cycles===\n");
  uint32_t i;
  istringstream(data_) >> i;
  printf("burst cycles %d\n", i);
  hw->piezo_set_burst_cycles(i);

}
static bool xBIT_allow_in = 0;
void fpga_mode::allow_input(string data_){
  /*while(xBIT_allow_in);
  xBIT_allow_in = true;
  printf("\n-==allow_input===\n");
  int time_out_cnt = 0;
  unsigned int time_dat = 0;
  //uint32_t i;
  //istringstream(data_) >> i;
  //enable_input = i;
  if(mode == MODE_SLAVE){

    hw->allow_input_trigger();

    //hw.allow_input_trigger();
    //while(hw.rdy_to_read());
    for(int time_out_cnt = 0; time_out_cnt <= 4294967294; time_out_cnt++){
      if(!hw->rdy_to_read()){
        time_dat = hw->read_trigger_time();
        cout << "\nTIME:" <<  +time_dat ; //<< " clk count : " << hw->read_trigger_time2();
        break;
      }
      cout << "\nCNT:" <<  +time_out_cnt;
    }
    //for(time_out_cnt=0; time_out_cnt <= 4294967294; time_out_cnt++){
    //  if(hw->rdy_to_read())
    //    break;
    //}
    //cout << "\n time_out_cnt " << time_out_cnt;

    send_time_frame(time_dat);

    enable_input = false;
  }
  xBIT_allow_in = false;*/
}

/*
sync_enable = msg->start_ptp_sync;
hw->piezo_set_burst_cycles(msg->burst_cycles);
burst_enable = msg->start_burst;
if(msg->start_continiouse_mode)
  hw->start_US_out();
else
  hw->stop_US_out();
*/


//-------- void pub ...
void fpga_mode::send_time_frame(uint32_t time_){
  std::stringstream ss_buffer;
  //cout << "\n========================= SEND TIMEFRAME\n";
  //ss_buffer <<"[" << masterlist.front() << "]" << (uint32_t)time_;
  /*if(masterlist.size() == 0){
    cout << "\n MASTERLISTFRONT = EMPTY";
    ss_buffer <<"[" << -4 << "]" << time_;
    return;
  }else{
    //cout << "\n MASTERLISTFRONT = " << +masterlist.front();
    //ss_buffer <<"[" << +masterlist.front() << "]" << time_;
    //printf("masterlist front %d\n", masterlist.front());
    //pop_vec(masterlist);


  }*/
  ss_buffer <<"[" << +(int)current_master << "]" << time_;
  //cout << "\n time out " << ss_buffer.str();

  //printf("list %d\n", masterlist.size());
  //printf("time %d\n", time_);

  //cout << ss_buffer.str();
  //pop_vec(masterlist);

  //cout << ss_buffer.str();

  trans->push_pub(time_frame_topic,ss_buffer.str());
}



//==============================
//Interface
//==============================
void fpga_mode::start_conversation(){
  enable_input = false;
  if(mode_pub != mode){
      mode = mode_pub;
    if(mode == MODE_MASTER){
      fp_start_conv = &fpga_mode::master_init;  //default &fpga_mode::slave_init
    } else {
      fp_start_conv = &fpga_mode::slave_init;  //default &fpga_mode::slave_init
    }

    ((*this).*(fp_start_conv))();
  }
}
void fpga_mode::conversation(){
  ((*this).*(fp_conv))();
}

//==============================
//Init routins
//==============================
void fpga_mode::master_init(){
  std::cout << "\nMASTER Init";

  fp_conv = &fpga_mode::master_conv;
  //for(uint8_t id_cnt = 0; id_cnt < MAX_CLIENTS; id_cnt++)
 //    system_sub[id_cnt] = nh->subscribe("/triangulation/" + std::to_string(id_cnt) + "/trigger_time", 1, &fpga_mode::get_slav_time, this);

  //system_pub.publish(system_ctl_msg_pub);
}

void fpga_mode::slave_init(){
  std::cout << "\nSLAVE Init";
  fp_conv = &fpga_mode::slave_conv;
  //system_pub  = nh->advertise<triangulation_msg::time_msg>("/triangulation/" + std::to_string(id) + "/time_data", 1);
  //system_sub[MAX_CLIENTS] = nh->subscribe("/triangulation/all/ctl", 1, &fpga_mode::get_syst_ctl, this); //Todo add sub function
  //system_pub.publish(time_msg_pub);

}

//==============================
//conversation routin
//==============================
void fpga_mode::master_conv(){
  cout << "\nstart master conversation: ";
  trans->push_pub("/time/set_zero","0");
  //enable slave
  //system_ctl_msg_pub.enable_slave_input=true;
  //master_pub.publish(system_ctl_msg_pub);
  //cout<<"\nTODO enable slave";
  //TODO: wait for response over ROS;
  //...
  //hw->start_US_out();

  hw->piezo_burst_out();
  send_time_frame(hw->US_start_time);


  //push_vec(time_msg_pub.trigger_time, hw->US_start_time);
  //push_vec(time_msg_pub.master_identifier, id); //master id

  //system_pub.publish(time_msg_pub);


  //usleep(10000);//Todo optimze this line
  //hw->stop_US_out();
}

//-------------------------------
void fpga_mode::slave_conv(){
  //std::cout << "\nstart slave conversation: ";
  while(xBIT_allow_in);
  xBIT_allow_in = true;
  //printf("\n-==allow_input===\n");
  int time_out_cnt = 0;
  unsigned int time_dat = 0;
  //uint32_t i;
  //istringstream(data_) >> i;
  //enable_input = i;
  if(mode == MODE_SLAVE){

    hw->allow_input_trigger();

    //hw.allow_input_trigger();
    //while(hw.rdy_to_read());
    for(int time_out_cnt = 0; time_out_cnt <= 4294967294; time_out_cnt++){
      if(!hw->rdy_to_read()){
        //time_dat = hw->read_trigger_time();
        time_dat = hw->read_trigger_time();
        //cout << "\nTIME:" <<  +time_dat ; //<< " clk count : " << hw->read_trigger_time2();
        break;
      }
      //cout << "\nCNT:" <<  +time_out_cnt;
    }
    //for(time_out_cnt=0; time_out_cnt <= 4294967294; time_out_cnt++){
    //  if(hw->rdy_to_read())
    //    break;
    //}
    //cout << "\n time_out_cnt " << time_out_cnt;

    send_time_frame(time_dat);

    enable_input = false;
  }
  xBIT_allow_in = false;


  //TODO: wait till slave allow;
  //....
/*  uint32_t input_delay_cnt = 0;
  for(;;){
    if(enable_input == true)
      break;
    input_delay_cnt++;
    if(input_delay_cnt >=  4294967294)
      return;
  }
  //while(enable_input != true);
  //if(enable_input == true)
  hw->allow_input_trigger(); //TODO ... do it via ros
  for(int time_out_cnt = 0; time_out_cnt <= 4294967294; time_out_cnt++){
    if(hw->rdy_to_read())
      break;
  }

  send_time_frame(hw->read_trigger_time());

  enable_input = false;*/
  //send_time_frame(uint32_t time_);
  //push_vec(time_msg_pub.trigger_time, hw->read_trigger_time());
  //push_vec(time_msg_pub.master_identifier, current_master_id); //master id

  //system_pub.publish(time_msg_pub);
}

void fpga_mode::set_time(uint32_t time_){
  hw->set_time(time_);
}


//==============================
//common routin for ros
//==============================
/*void fpga_mode::get_mode(const triangulation_msg::master_list::ConstPtr& msg){
    .... msg->master_id_list;


    ROS_INFO("current mode: %d", msg->mode);
    mode_pub = msg->mode;
    sync_mode = msg->sync_mode;
    sync_enable = msg->sync_enable;
    ROS_INFO("sync enable: %d", msg->sync_enable);

}*/




void fpga_mode::pop_vec(vector<uint8_t>& vec){
  for (uint8_t vec_cnt = 0; vec_cnt < vec.size()/2; vec_cnt++){
    int buff;
    buff = vec[vec.size()-1-vec_cnt];
    vec[vec.size()-1-vec_cnt]=vec[vec_cnt];
    vec[vec_cnt]=buff;
  }
  vec.pop_back();
  //vec.resize(MAX_MASTER_LIST_SIZE);
  for (uint8_t vec_cnt = 0; vec_cnt < vec.size()/2; vec_cnt++){
    int buff;
    buff = vec[vec.size()-1-vec_cnt];
    vec[vec.size()-1-vec_cnt]=vec[vec_cnt];
    vec[vec_cnt]=buff;
  }
}

//ring bufferd vector push
void fpga_mode::push_vec(vector<float>& vec, float data){
  if(vec.size() < MAX_MASTER_LIST_SIZE){
    vec.push_back(data);
    return;
  }
  for (uint8_t vec_cnt = 0; vec_cnt < vec.size()/2; vec_cnt++){
    int buff;
    buff = vec[vec.size()-1-vec_cnt];
    vec[vec.size()-1-vec_cnt]=vec[vec_cnt];
    vec[vec_cnt]=buff;
  }
  vec.resize(MAX_MASTER_LIST_SIZE);
  for (uint8_t vec_cnt = 0; vec_cnt < vec.size()/2; vec_cnt++){
    int buff;
    buff = vec[vec.size()-1-vec_cnt];
    vec[vec.size()-1-vec_cnt]=vec[vec_cnt];
    vec[vec_cnt]=buff;
  }

  vec.push_back(data);
}

void fpga_mode::push_vec(vector<uint8_t>& vec, uint8_t data){
  if(vec.size() < MAX_MASTER_LIST_SIZE){
    vec.push_back(data);
    return;
  }
  for (uint8_t vec_cnt = 0; vec_cnt < vec.size()/2; vec_cnt++){
    int buff;
    buff = vec[vec.size()-1-vec_cnt];
    vec[vec.size()-1-vec_cnt]=vec[vec_cnt];
    vec[vec_cnt]=buff;
  }

  vec.resize(MAX_MASTER_LIST_SIZE-1);

  //vec.resize(MAX_MASTER_LIST_SIZE-1);
  for (uint8_t vec_cnt = 0; vec_cnt < vec.size()/2; vec_cnt++){
    int buff;
    buff = vec[vec.size()-1-vec_cnt];
    vec[vec.size()-1-vec_cnt]=vec[vec_cnt];
    vec[vec_cnt]=buff;
  }

  vec.push_back(data);
}
