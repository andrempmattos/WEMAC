/**
 * \file Main vending machine class 
 * \brief It holds all the core system
 * 
 * \author André Mattos <andrempmattos@gmail.com>
 * \author Daniel Baron <zdaniz22@gmail.com>
 * 
 * \date 10/27/2017
 * 
 * \defgroup VendingMachineCore
 */

#include "VendingMachine.hpp"
#include <iostream>

using namespace VMCore;

VendingMachine::VendingMachine(Interface* t_interfaceOverride) : StateMachine(ST_MAX_STATES) {

    //Log system setup
    logVendingMachine.setLevel(Log::levelError);
    logVendingMachine.setScope("[VMCORE]");
    logVendingMachine.warn("(CONSTRUCTOR)VendingMachine");

    //Change the interface used
    m_interface = t_interfaceOverride;

    //Add the product database into the system
    productDatabase = new Product[Product::MAX_VM_SLOTS];

    std::vector<productInfo> pData(Product::MAX_VM_SLOTS);
    productDatabase->getProductDatabase(pData);
    
    productDatabase->setProductDatabase(productDatabase, pData);

    logVendingMachine.debug("Product database ready");    
} 

//Timer VendingMachine external event
void VendingMachine::timerEvent(void) {
    //Given the timer event, transition to a new state based upon 
    //the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (ST_ADVERTISING)  // ST_Idle
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Devolution
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Validation
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Transaction
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Deployment
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Advertising
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Confirmation
    END_TRANSITION_MAP(nullptr)
}

//Cancel VendingMachine external event
void VendingMachine::cancelEvent(void) {
    //Given the cancel event, transition to a new state based upon 
    //the current state of the state machine
    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Idle
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Devolution
        TRANSITION_MAP_ENTRY (ST_DEVOLUTION)  // ST_Validation
        TRANSITION_MAP_ENTRY (ST_DEVOLUTION)  // ST_Transaction
        TRANSITION_MAP_ENTRY (ST_DEVOLUTION)  // ST_Deployment
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Advertising
        TRANSITION_MAP_ENTRY (ST_DEVOLUTION)  // ST_Confirmation
    END_TRANSITION_MAP(nullptr)
}

//Increment VendingMachine cash external event
void VendingMachine::cashIncrementEvent(float t_inputCash) {
    VendingMachineData* pData = new VendingMachineData();
    pData->cashValue = t_inputCash;

    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (ST_TRANSACTION) // ST_Idle
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Devolution
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Validation
        TRANSITION_MAP_ENTRY (ST_TRANSACTION) // ST_Transaction
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Deployment
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Advertising
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Confirmation
    END_TRANSITION_MAP(pData)
}

//Select VendingMachine product external event
void VendingMachine::productSelectionEvent(int t_productSelection) {
    VendingMachineData* pData = new VendingMachineData();
    pData->productSelection = t_productSelection;

    BEGIN_TRANSITION_MAP                      // - Current State -
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Idle
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Devolution
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Validation
        TRANSITION_MAP_ENTRY (ST_VALIDATION)  // ST_Transaction
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Deployment
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Advertising
        TRANSITION_MAP_ENTRY (EVENT_IGNORED)  // ST_Confirmation
    END_TRANSITION_MAP(pData)
}

//State machine sits here when there is any user interaction
void VendingMachine::ST_Idle(EventData* pData) {
    logVendingMachine.warn("(STATE)Idle");

    AdvertisingData advertising;
    advertising.advertisingOutput = "Propaganda $$$";
    m_interface->printAdvertising(&advertising);
}

//Give back the current user cash 
void VendingMachine::ST_Devolution(EventData* pData) {
	logVendingMachine.warn("(STATE)Devolution");
    logVendingMachine.info(("(DEVOLUTION)" + std::to_string(m_transactionCash)));

    UserData user;
    user.userOutput = "Devolution: $" + std::to_string(m_transactionCash);
    m_interface->setUserOutput(&user);
    
    m_transactionCash = 0;
    
    InternalEvent(ST_IDLE);
}

//Check if the transaction is valid due to the user product selection 
void VendingMachine::ST_Validation(VendingMachineData* pData) {
    logVendingMachine.warn("(STATE)Validation");
    
       
    UserData user;  
    bool isPurchaseValid = (m_transactionCash >= productDatabase[pData->productSelection].getValue()) ? true : false;

    if (isPurchaseValid) {
        VendingMachineData* pDataTemp = new VendingMachineData();
        pDataTemp->productSelection = pData->productSelection; 

        logVendingMachine.info(("(PRODUCT)" + productDatabase[pData->productSelection].getName() + " | Valid purchase"));
        InternalEvent(ST_CONFIRMATION, pDataTemp);  // Valid Event
    }
    else {
        user.userOutput = "Insufficient balance, Try put more credit";
        m_interface->setUserOutput(&user);

        logVendingMachine.info(("(PRODUCT)" + productDatabase[pData->productSelection].getName() + " | Invalid purchase"));
        VendingMachineData* pDataNull = new VendingMachineData();
        InternalEvent(ST_TRANSACTION, pDataNull);   //Invalid Event
    }
}
 
//Increase the user cash due to input coins
void VendingMachine::ST_Transaction(VendingMachineData* pData) {
	logVendingMachine.warn("(STATE)Transaction");
    m_transactionCash += (pData->cashValue);
    logVendingMachine.info(("(TOTAL) $" + std::to_string(m_transactionCash)));

    UserData user;
    user.userOutput = "Current cash: $" + std::to_string(m_transactionCash);
    m_interface->setUserOutput(&user);    
}

//Deploy the selected product to the user
void VendingMachine::ST_Deployment(VendingMachineData* pData) {
    logVendingMachine.warn("(STATE)Deployment");
    logVendingMachine.info(("(PRODUCT)" + productDatabase[pData->productSelection].getName() + " | Deploying"));
    
    UserData user;
    user.userOutput = "Deploying: " + productDatabase[pData->productSelection].getName()
                        + " $" + std::to_string(productDatabase[pData->productSelection].getValue());
    m_interface->setUserOutput(&user);

    SystemData system;
    system.systemOutput = "(Event)Product deployment";
    m_interface->setSystemOutput(&system);

    InternalEvent(ST_DEVOLUTION);
}

//Show same Advertisings
void VendingMachine::ST_Advertising(void) {
    logVendingMachine.warn("(STATE)Advertising");

    std::cout << "state advertising" << std::endl;


    InternalEvent(ST_IDLE);
}

//Confirmation State
void VendingMachine::ST_Confirmation(VendingMachineData* pData) {
    logVendingMachine.warn("(STATE)Confirmation");

    UserData user;
    user.userOutput = "Do you confirm the purchase? Item: " + productDatabase[pData->productSelection].getName()
                        + " $" + std::to_string(productDatabase[pData->productSelection].getValue());
    m_interface->setUserOutput(&user);
    m_interface->getUserInput(&user);

    bool isConfirmed = (user.userInput == "y") ? true : false;

     if (isConfirmed) {
        VendingMachineData* pDataTemp = new VendingMachineData();
        pDataTemp->productSelection = pData->productSelection;
        m_transactionCash -= productDatabase[pData->productSelection].getValue(); 

        InternalEvent(ST_DEPLOYMENT, pDataTemp);
    }
    else {

        VendingMachineData* pDataNull = new VendingMachineData();
        InternalEvent(ST_TRANSACTION, pDataNull);
    }  
}