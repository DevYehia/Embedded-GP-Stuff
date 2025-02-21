// can_instance_t* can_instance;


// void Can_init(can_instance_t* can_pal_instance, can_user_config_t* can_pal_Config)
// {
//     can_instance = can_pal_instance;
//     CAN_Init(can_pal_instance, can_pal_Config);
//     CAN_InstallEventCallback(can_pal_instance, interrupt_callback, NULL);
//     can_buff_config_t buffConf = {false, false, 0xAA, CAN_MSG_ID_STD, false};
//     CAN_ConfigRxBuff(&can_pal1_instance, RX_BUFF_NUM, &buffConf, 0x3);
//     CAN_Receive(&can_pal1_instance, RX_BUFF_NUM, &recvMessage);
// }

// void CanSend(can_message_t * message)
// {
//     CAN_Send(can_instance,buffIdx,message);
// }