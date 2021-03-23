#pragma once

// comentei events.
// #include "ofEvents.h"
#include "ofxMicroUI.h"
#include "ofxOsc.h"

#if defined( TARGET_OF_IPHONE ) || defined( TARGET_OF_IOS ) || defined( TARGET_ANDROID )
#define MICROUI_TARGET_TOUCH
#endif

#ifdef MICROUI_TARGET_TOUCH
#include "ofxAccelerometer.h"
#endif

class ofxMicroUIRemote : public ofBaseApp {
public:
	
	enum bundleUsage {
		USEBUNDLE_NO,
		USEBUNDLE_YES,
		USEBUNDLE_ALL,
	};
	
	bundleUsage useBundle = USEBUNDLE_YES;
	
//	bool useBundle = true;
	
	// change to ofxMicroUI Alerts
	struct alert {
		public:
		string msg;
		int tempo = 300;
		float alpha = 255;
		bool ok = true;
		alert(string m) : msg(m) { 
		}

		void draw() {
			if (tempo < 0) {
				ok = false;
			} else {
				tempo -= 4;
				alpha = ofClamp(tempo, 0, 255);
				ofSetColor(40, alpha);
				ofDrawRectangle(0,0,180,20);
				ofSetColor(255, alpha);
				// ofDrawBitmapString(msg + ":" +ofToString(tempo), 20, 18);
				ofDrawBitmapString(msg, 17, 15);
			}
		}
	};

	vector <alert> alerts;
	vector <int> willErase;

	void drawAlerts() {
		ofPushMatrix();

		ofSetColor(255);
		ofDrawBitmapString(ofToString(alerts.size()), 20, 18);
		ofTranslate(0, 18);
        for (int a=alerts.size()-1; a>=0; a--) {
//		for (int a=0; a<alerts.size(); a++) {
			if (alerts[a].ok) {
				ofTranslate(0, 18);
				alerts[a].draw();
			} else {
				willErase.push_back(a);
			}
		}
		ofPopMatrix();

		for (auto w : willErase) {
			alerts.erase(alerts.begin() + w);
		}
		willErase.clear();
 	}

	void alert(string s) {
		alerts.emplace_back(s);
	}


	// novidade 2021.
	ofxMicroUI * _uiMaster = NULL;

	ofxMicroUI u;
	// no caso de interface remota o pointer _ui aponta pra u que é ela propria.
	ofxMicroUI * _ui = &u;
	string configFile = "";

	ofxMicroUI::inspector * oscInfo = NULL;
	ofxMicroUI::inspector * oscInfoReceive = NULL;
    
    ofxMicroUI::inspector * oscIP = NULL;
    

	bool sendOnLoadPreset = true;

	ofxOscSender 	send;
	ofxOscReceiver 	receive;

	string 	serverAddress = "";
	int 	serverPort = 9000;
	string 	remoteAddress = "";
	int 	remotePort = 8000;
	
	ofxOscBundle bundle;
	
	ofEvent<string> eventMessage;
	map <string, ofxMicroUI *> _nameUIs;
	
	map <string, std::function<void()>> msgFunction;
	
	string reservedAddress = "/reservedAddress/";
	
	bool isServer = true;
	
	void uiEventMaster(string & s) {
//		cout << "SETUP REMOTE " << s << endl;
		if (s == "setup") {
			if (configFile != "") {
				loadConfig(configFile);
				
				if (isServer) {
					setupServer();
				}
				else {
					setupRemote();
				}
			}
		}
	}
	
	void loadConfig(string file) {
//		 cout << "loadConfig " << file << endl;
		if (ofFile::doesFileExist(file)) {
			map <string, string> configs = ofxMicroUI::loadConfigPairs(file);
			if (configs.count("remotePort")) {
				remotePort = ofToInt(configs["remotePort"]);
			}
			
			if (configs.count("serverPort")) {
				serverPort = ofToInt(configs["serverPort"]);
			}
			
			if (configs.count("remoteAddress")) {
				remoteAddress = configs["remoteAddress"];
			}
			
			if (configs.count("serverAddress")) {
				serverAddress = configs["serverAddress"];
			}
			
			if (configs.count("useBundle")) {
				if (configs["useBundle"] == "no") {
					useBundle = USEBUNDLE_NO;
				}
				else if (configs["useBundle"] == "yes") {
					useBundle = USEBUNDLE_YES;
				}
				else if (configs["useBundle"] == "all") {
					useBundle = USEBUNDLE_ALL;
				}
//				serverAddress = configs["serverAddress"];
			}
			
			if (configs.count("server")) {
				isServer = true;
			}
			
			if (configs.count("remote")) {
				isServer = false;
			}
			
			if (configs.count("addUIByTag")) {
				addUIByTag(configs["addUIByTag"]);
			}
			
            if (configs.count("addAllUIs")) {
                addAllUIs();
            }
			
            if (configs["addUIByNames"] != "") {
                addUIByNames(configs["addUIByNames"]);
            }
		}
	}
	
	void setup() {
		ofAddListener(_uiMaster->uiEventMaster, this, &ofxMicroUIRemote::uiEventMaster);
		// ofAddListener(ofEvents().draw, this, &ofxMicroUIRemote::onDraw);
		ofAddListener(ofEvents().update, this, &ofxMicroUIRemote::onUpdate);
	}
	
	ofxMicroUIRemote(ofxMicroUI * _ui, string f) : _uiMaster(_ui), configFile(f) {
//		loadConfig(f);
		setup();
	}

	ofxMicroUIRemote(string f) : configFile(f) {
//		loadConfig(f);
		setup();
	}

	ofxMicroUIRemote() {
		setup();
	}
	
	~ofxMicroUIRemote() {}
	





	void addUI(ofxMicroUI * ui) {
//		cout << "addUI " << ui->uiName << endl;
		_nameUIs[ui->uiName] = ui;
		ofAddListener(_nameUIs[ui->uiName]->uiEvent, this, &ofxMicroUIRemote::uiEvent);
		ofAddListener(_nameUIs[ui->uiName]->uiEventMaster, this, &ofxMicroUIRemote::uiEventString);
	}
	// fazer um addAllUis
	
	void addAllUIs() {
		for (auto & u : _uiMaster->allUIs) {
            // cout << "|||| ADD " << u->uiName << endl;
			addUI(u);
		}
	}

	void addUIByTag(string tags) {
        for (auto tag : ofSplitString(tags, ",")) {
            cout << "ADDUIBYTAG " << tag << endl;
            for (auto & u : _uiMaster->allUIs) {
                if (u->uiTag == tag) {
                    cout << u->uiName <<endl;
                    addUI(u);
                }
            }
        }
	}
    
    void addUIByNames(string s) {
        vector <string> names = ofSplitString(s, ",");
        for (auto & n : names) {
            addUI(&_uiMaster->uis[n]);
        }
    }
	
    void setupAll() {
        if (oscIP != NULL) {
            string ip = "remote:" + remoteAddress + ":" + ofToString(remotePort);
            oscIP->set(ip);
        }
    }
    
	void setupServer() {
		receive.setup(serverPort);
		bool serverIsSetup;
		
		try {
			serverIsSetup = send.setup(remoteAddress, remotePort);
//			cout << "trying " << remoteAddress << ":" << remotePort << endl;
		} catch (exception){
			cout << "ofxMicroUIRemote :: &&& no internet &&&" << endl;
		}
		if (serverIsSetup) {
			string message = "ofxMicroUIRemote server \r" 
			+ string("server = ") + serverAddress + ":" + ofToString(serverPort) + "\r"
			+ string("remote = ") + remoteAddress + ":" + ofToString(remotePort) ; //+ "\r"
//			 cout << message << endl;
			ofxMicroUI::messageBox(message);
		}
        
        setupAll();
	}
	
	void setupRemote() {
		try {
			receive.setup(remotePort);
			string message = "ofxMicroUIRemote REMOTE \r";
			+ "Port = " + ofToString(remotePort);
			ofxMicroUI::messageBox(message);
		} catch (const exception){
			cout << "ofxMicroUIRemote :: &&& no internet &&&" << endl;
		}
        
        setupAll();
	}
	
	void onDraw(ofEventArgs &data) { 
		// draw(); 
	}

	void onUpdate(ofEventArgs &data) {
		if (bundle.getMessageCount()) {
			send.sendBundle(bundle);
			bundle.clear();
		}
		
		// update();
		while(receive.hasWaitingMessages()){
			ofxOscMessage m;
			receive.getNextMessage(m);
			alert(m.getAddress());
			cout << "receiving message :: " + m.getAddress() << endl;

			string msg = m.getAddress();
			ofNotifyEvent(eventMessage, msg);
			
			if (oscInfoReceive != NULL) {
				oscInfoReceive->set(m.getAddress());
			}

			vector <string> addr = ofSplitString(m.getAddress(), "/");
//			cout << addr.size() << endl;
			
			if (m.getAddress() == "/reservedAddress/createFromText") {
				cout << "YOO" << endl;
				ofBuffer blob = m.getArgAsBlob(0);
				_ui->clear();
				
				_ui->initFlow();
					//alert("createFromText " + fileName);
				if (!_ui->hasListeners) {
					_ui->addListeners();
				}
				
				// vector <string> lines;
				// for(auto & line: blob.getLines()) {
				// 	lines.push_back(line);
				// }
				string lines = blob.getText();
				_ui->createFromLines(lines);
                
                cout << blob.getText() << endl;
                cout << "UINAME = " << _ui->uiName << endl;

				// string allLines = ofJoinString(lines, "\r");
				// _ui->createFromLines(allLines);
					
//				string s = "createFromText";
//				ofNotifyEvent(_ui->uiEventMaster, s);

				// NOVIDADE 14 jul 2020
				_ui->redrawUI = true;
				
				cout << "elements size :: " << _ui->elements.size() << endl;
//					_ui->autoFit();
			}
			
			if (addr.size() >= 3) {
				// cout << addr.size() << endl;
				string uiName = addr[1];
				string name = addr[2];
				// prova de conceito mas eventualmente nao vai funcionar ainda por causa do propagateevent. refazer isso logo em breve
				
				if ( _nameUIs.find(uiName) != _nameUIs.end() ) {
					ofxOscArgType k = m.getArgType(0);
					_nameUIs[uiName]->_settings->eventFromOsc = true;
					
					if (k == OFXOSC_TYPE_FLOAT) {
						cout << "FLOAT" << endl;
						_nameUIs[uiName]->set(name, (float) m.getArgAsFloat(0));
					}
					else if (k == OFXOSC_TYPE_INT32 || k == OFXOSC_TYPE_INT64) {
						cout << "INT" << endl;
						_nameUIs[uiName]->set(name, (int) m.getArgAsInt(0));
					}
					else if (k == OFXOSC_TYPE_FALSE) {
						cout << "BOOL FALSE" << endl;
						_nameUIs[uiName]->set(name, (bool) false);
					}
					else if (k == OFXOSC_TYPE_TRUE) {
						cout << "BOOL TRUE" << endl;
						_nameUIs[uiName]->set(name, (bool) true);
					}
					else if (k == OFXOSC_TYPE_STRING) {
						cout << "STRING" << endl;
						_nameUIs[uiName]->set(name, m.getArgAsString(0));
					}
					
					_nameUIs[uiName]->_settings->eventFromOsc = false;
					cout << "-------" << endl;
				}
			}
		}
	}
	
	//--------------------------------------------------------------
	void uiEvent(ofxMicroUI::element & e) {
		cout << e._ui->uiName << " : " << e.name << endl;
//		cout << sendOnLoadPreset << endl;
//		cout << e._settings->presetIsLoading << endl;
//		cout << "-----" << endl;
	
		if (e._settings->eventFromOsc) {
			// cout << "EVENT RECEIVED FROM OSC :: ignoring forward " << e.name << endl;
		} else {


			if (sendOnLoadPreset || !e._settings->presetIsLoading) {
				string address = "/" + e._ui->uiName + "/" + e.name;
				
				// transformar em bundle aqui
		//		cout << "MSG " << address << endl;

				ofxOscMessage m;
				m.setAddress(address);

				if (ofxMicroUI::slider * els = dynamic_cast<ofxMicroUI::slider*>(&e)) {
					if (els->isInt) {
						m.addIntArg(*e.i);
					} else {
						m.addFloatArg(*e.f);
					}
				}
				
				else if (dynamic_cast<ofxMicroUI::toggle*>(&e)) {
					m.addBoolArg(*e.b);
				}

				else if (dynamic_cast<ofxMicroUI::radio*>(&e)) {
					m.addStringArg(*e.s);
				}
				
				else if (dynamic_cast<ofxMicroUI::inspector*>(&e) || dynamic_cast<ofxMicroUI::bar*>(&e)) {
					m.addStringArg(*e.s);
				}
				
				if (m.getNumArgs() > 0) {
					if (e._ui->_settings->presetIsLoading) {
						if (useBundle == USEBUNDLE_NO) {
							send.sendMessage(m, false);
						} else {
							bundle.addMessage(m);
						}
					} else {
						if (useBundle == USEBUNDLE_ALL) {
							bundle.addMessage(m);
						} else {
							send.sendMessage(m, false);
						}
						if (oscInfo != NULL) {
							if (!e._settings->presetIsLoading)
                            {
								oscInfo->set(address);
							}
						}
					}
				}
			}
		}
	}


	ofxMicroUI * _mirrorUI = NULL;
	void addMirrorUI(ofxMicroUI * ui) {
		_mirrorUI = ui;
		ofAddListener(_nameUIs[ui->uiName]->uiEventMaster, this, &ofxMicroUIRemote::uiEventMirror);
	}
	
	void uiEventMirror(string & e) {
		cout << "event mirror" << e << endl;
		// temporario por enquanto. nao sabemos ainda de que UI vem.
		if (e == "createFromText") {
			cout << e << endl;	
			ofxOscMessage m;
			m.setAddress(reservedAddress + "createFromText");

			ofBuffer blob;
			blob.append("clear\r");
			blob.append("uiName\t" + _mirrorUI->uiName +"\r");

			blob.append(_mirrorUI->createdLines);
			cout << "REMOTE OSC createFromText :: " << endl;
			cout << _mirrorUI->createdLines << endl;
			m.addBlobArg(blob);
			send.sendMessage(m, false);
		}
	}

	void uiEventString(string & e) {
//		cout << "remote event " << e << endl;
		// temporario por enquanto. nao sabemos ainda de que UI vem.
		if (e == "createFromText") {
			cout << e << endl;	
			ofxOscMessage m;
			m.setAddress(reservedAddress + "createFromText");

			ofBuffer blob;
//			if (clear) {
//				blob.append("uiClear\n");
//			}

//			if (remoteStyle != "") {
//				blob.append(remoteStyle);
//			}
			
//			if (ofFile::doesFileExist("uiRemote.txt")) {
//				blob.append(ofBufferFromFile("uiRemote.txt"));
//			}

			// this will continue after the event.
			if (2==3) 
			{

				blob.append(_nameUIs["master"]->createdLines);
				cout << "REMOTE OSC createFromText :: " << endl;
				cout << _nameUIs["master"]->createdLines << endl;
				m.addBlobArg(blob);
				send.sendMessage(m, false);
			}

			
			//			bundle.addMessage(m);
		}
	}
};
