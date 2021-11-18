#include "gui_netplay.h"
#include "netplay_session.h"
#include "netplay_connection.h"
#include "input_recv_patch.h"
#include "async_input_grab.h"
#include "constant_random_patch.h"
#include "antipause_patch.h"
#include "preconditions.h"
#include "temple_anticrash.h"
#include "disable_ctrlmenu_patch.h"

//DEBUG
#include "save_manager.h"
#include "gui.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Check_Button.H>


namespace NetplayGUI 
{
	class ActionButton : public Fl_Button {
	private:
		std::function<void(int)> action;

	public:
		ActionButton(int x, int y, int w, int h, char* L, std::function<void(int)> action) : 
			Fl_Button(x,y,w,h,L), action(action) 
		{}

		virtual int handle(int evt) override {
			action(evt);
			return Fl_Button::handle(evt);
		}
	};

	std::function<void(bool)> active_cb;
	std::function<void(bool)> visib_cb;

	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<Seeder> seeder;
	std::shared_ptr<GameHooks> hooks;


	std::shared_ptr<NetplaySession> session;
	std::shared_ptr<ConnectHandle> connect_handle;
	std::shared_ptr<HostHandle> host_handle;
	std::shared_ptr<InputReceivePatch> irp;
	std::shared_ptr<ConstantRandomPatch> crp;
	std::shared_ptr<TempleAnticrash> anticrash;
	std::shared_ptr<DisableCtrlMenuPatch> dcmp;

	std::shared_ptr<SaveManager> sm;


	Fl_Window* netplay_window = nullptr;

	//++++++++++ hosting
	Fl_Input* host_port = nullptr;
	ActionButton* host_btn = nullptr;
	bool host_cancelled = false;
    Fl_Check_Button* host_own_camera = nullptr;
	//==========

	//++++++++++ connecting
	Fl_Input* client_host = nullptr;
	Fl_Input* client_port = nullptr;
	Fl_Check_Button* client_own_camera = nullptr;
	ActionButton* connect_btn = nullptr;
	//==========

	//++++++++++ input buffer config
	Fl_Input* buffer_max = nullptr;
	ActionButton* set_buffer_max = nullptr;
	//==========

	void netplay_status(const std::string& status) {
		if(status.empty()) {
			netplay_window->label("Netplay");
		}
		else {
			netplay_window->label((std::string("Netplay - ") + status).c_str());
		}
	}

	void init_window() 
	{	
		Fl_Window* o = new Fl_Window(256, 146+77, "Netplay");
		netplay_window = o;
	

		netplay_window->callback([](Fl_Widget* widget) {
			if(host_handle || connect_handle || session) {
				return;
			}

			Fl_Window* w = (Fl_Window*)widget;
			if(visib_cb) {
				if(w->visible()) {
					w->hide();
					visib_cb(false);
				}
				else {
					w->show();
					visib_cb(true);
				}
			}
		});

		{ 
			Fl_Group* o = new Fl_Group(5, 8, 245, 101);
			o->box(FL_UP_FRAME);
			o->end();
		}

		{ Fl_Group* o = new Fl_Group(5, 60+56, 245, 101);
			o->box(FL_UP_BOX);
			o->end();
		}

		host_port = new Fl_Input(50, 19, 105, 24, "Port:");
		host_port->value("5394");

		//!TODO implement preconditions for host / connect buttons (have to be on main menu, opop->controller_count() == 2, load proper save file, etc.)

		host_btn = new ActionButton(160, 18, 80, 25, "Host", [=](int evt) 
		{
			if(evt == 2) 
			{
				auto cancel = [=]() {
					connect_btn->activate();
					host_port->activate();
					netplay_status("");
					host_btn->activate();
					host_btn->label("Host");
					host_handle = nullptr;
					connect_handle = nullptr;
					netplay_window->redraw();
					active_cb(false);
				};

				if(session) {
					session->close(NS_REQUESTED);
					session = nullptr;
					cancel();
					return;
				}

				int npp_err;
				NetplayPreconditions pre(dp, hooks);
				if(npp_err = pre.check()) {
					MessageBox(NULL, NetplayPreconditions::FriendlyString(npp_err).c_str(), "Please correct the following", MB_OK);
					return;
				}

				if(!host_handle) 
				{
					try 
					{
						netplay_status("Waiting for connection..");
						host_btn->deactivate();
						connect_btn->deactivate();
						host_btn->label("Cancel");

						sm->load_netplay_save([=](bool result) 
						{
							host_btn->activate();

							if(!result) {
								MessageBox(NULL, "Error: Frozlunky failed to write the netplay savefile, desync may occur.", "Netplay", MB_OK);
							}

							dcmp->perform();
							crp->perform();
							anticrash->perform();

							std::shared_ptr<InputPushBuilder> net_ipb = std::make_shared<InputPushBuilder>(1, dp, hooks);
							std::shared_ptr<InputPushBuilder> local_ipb = std::make_shared<InputPushBuilder>(0, dp, hooks);
							net_ipb->perform();
							local_ipb->perform();

							irp->set_input_push(1, net_ipb);
							irp->set_input_push(0, local_ipb);

							irp->perform();

							host_handle = NetplayConnection::Host(std::stoi(host_port->value()), [=](std::shared_ptr<NetplayConnection> conn) 
							{
								if(!conn) {
									irp->undo();
									crp->undo();
									dcmp->undo();
									anticrash->undo();
									net_ipb->undo();
									local_ipb->undo();
									cancel();
									if(!host_cancelled) {
										MessageBox(NULL, "An error occurred when starting the server.", "Netplay Hosting Error", MB_OK);
									}
									host_cancelled = false;
									return;
								}

								conn->on_disconnect([=](NetplayDisconnectEvent evt) {
									crp->undo();
									anticrash->undo();
									session = nullptr;
									cancel();
									netplay_status("");
									MessageBox(NULL, (std::string("Connection closed: ") + NDFriendlyString(evt)).c_str(), "Connection closed", MB_OK);
								});

								std::shared_ptr<OwnCameraPatch> own_camera;
								if (host_own_camera->value()) {
									own_camera = std::make_shared<OwnCameraPatch>(0, spel);
                                    own_camera->perform();
								}

								session = std::shared_ptr<NetplaySession>(new NetplaySession(0, net_ipb, local_ipb, conn, irp, seeder, hooks, dp, own_camera));

								try {
									session->set_input_buffer_max(std::stoi(buffer_max->value()));
								}
								catch(std::exception e) {
									session->set_input_buffer_max(INPUT_BUFFER_MAX_DEFAULT);
								}

								session->ping_info([=](int ms) {
									netplay_status("Ping: " + std::to_string(ms) + "ms");
								});

								session->buf_change_cb([=](int buf) {
									buffer_max->value(std::to_string(buf).c_str());
								});

								conn->start();

								netplay_status("Connected");
								host_btn->activate();
								host_btn->label("Disconnect");
								host_handle = nullptr;
							});

							active_cb(true);
						});
					}
					catch(std::exception e) {
						cancel();
						MessageBox(NULL, (std::string("Error: " ) + e.what()).c_str(), "Netplay Error", MB_OK);
					}
				}
				else
				{
					host_cancelled = true;
					host_handle->set_killed(true);
					active_cb(false);
				}
			}
		});

		buffer_max = new Fl_Input(50, 52, 105, 24, "Buf:");
		buffer_max->value(std::to_string(INPUT_BUFFER_MAX_DEFAULT).c_str());

		set_buffer_max = new ActionButton(160, 52, 80, 25, "Set", [=](int evt) {
			if(evt == 2)
			{
				if(session && !session->is_closed()) {
					try {
						session->set_input_buffer_max(std::stoi(buffer_max->value()));
					}
					catch(std::exception e) {
						MessageBox(NULL, "Invalid max buffer value.", "Error", MB_OK);
					}
				}
			}
		});
		
		host_own_camera = new Fl_Check_Button(46, 80, 155, 25, "Enable own camera");
		
		////////////////////
		// Client Connect //
		////////////////////
		
		client_host = new Fl_Input(47, 69+56, 195, 26, "Host:");
		client_port = new Fl_Input(46, 103+56, 105, 25, "Port:");
		client_port->value("5394");

		connect_btn = new ActionButton(160, 104+56, 80, 25, "Connect", [=](int evt) 
		{
			if(evt == 2) 
			{
				auto revert = [=]() {
					netplay_status("");
					connect_btn->activate();
					connect_btn->label("Connect");
					host_btn->label("Host");
					host_btn->activate();
					netplay_window->redraw();
					set_buffer_max->activate();
					buffer_max->activate();
					active_cb(false);
				};

				if(session) {
					session->close(NS_REQUESTED);
					session = nullptr;
					revert();
					return;
				}

				int npp_err;
				NetplayPreconditions pre(dp, hooks);
				if(npp_err = pre.check()) {
					MessageBox(NULL, NetplayPreconditions::FriendlyString(npp_err).c_str(), "Please correct the following", MB_OK);
					return;
				}

				try 
				{
					set_buffer_max->deactivate();
					buffer_max->deactivate();
					host_btn->deactivate();
					connect_btn->deactivate();
					netplay_status("Connecting to server..");

					sm->load_netplay_save([=](bool result)
					{
						if(!result) {
							MessageBox(NULL, "Error: Frozlunky failed to write the netplay savefile, desync may occur.", "Netplay", MB_OK);
						}

						dcmp->perform();
						crp->perform();
						anticrash->perform();

						std::shared_ptr<InputPushBuilder> net_ipb = std::make_shared<InputPushBuilder>(0, dp, hooks);
						std::shared_ptr<InputPushBuilder> local_ipb = std::make_shared<InputPushBuilder>(1, dp, hooks);
						net_ipb->perform();
						local_ipb->perform();

						irp->set_input_push(0, net_ipb);
						irp->set_input_push(1, local_ipb);

						irp->perform();

						std::string host = client_host->value();
						connect_handle = NetplayConnection::Connect(host, std::stoi(client_port->value()), [=](std::shared_ptr<NetplayConnection> conn) 
						{
							if(!conn) {
								irp->undo();
								crp->undo();
								dcmp->undo();
								anticrash->undo();
								net_ipb->undo();
								local_ipb->undo();
								netplay_status("");
								MessageBox(NULL, "Connection failed", "Netplay Error", MB_OK);
								revert();
								return;
							}

							conn->on_disconnect([=](NetplayDisconnectEvent evt) {
								crp->undo();
								anticrash->undo();
								session = nullptr;
								revert();
								MessageBox(NULL, (std::string("Connection closed: ") + NDFriendlyString(evt)).c_str(), "Connection closed", MB_OK);
                            });

                            std::shared_ptr<OwnCameraPatch> own_camera;
                            if (client_own_camera->value()) {
                                own_camera = std::make_shared<OwnCameraPatch>(1, spel);
                                own_camera->perform();
                            }
				
							session = std::shared_ptr<NetplaySession>(new NetplaySession(1, net_ipb, local_ipb, conn, irp, seeder, hooks, dp, own_camera));
							
							session->ping_info([=](int ms) {
								netplay_status("Ping: " + std::to_string(ms) + "ms");
							});

							session->buf_change_cb([=](int buf) {
								buffer_max->value(std::to_string(buf).c_str());
							});

							conn->start();

							netplay_status("Connected.");
							connect_btn->activate();
							connect_btn->label("Disconnect");
							connect_handle = nullptr;
						});

						active_cb(true);
					});
				}
				catch(std::exception e) {
					MessageBox(NULL, (std::string("Error: " ) + e.what()).c_str(), "Netplay Error", MB_OK);
				}
			}
		});

        client_own_camera = new Fl_Check_Button(46, 133 + 56, 155, 25, "Enable own camera");

		o->end();
	}

	void NetplayChangeCallback(std::function<void(bool)> acb) {
		active_cb = acb;
	}

	void VisibilityChangeCallback(std::function<void(bool)> vcb) {
		visib_cb = vcb;
	}

	void DisplayNetplayGUI() {
		if(netplay_window) {
			netplay_window->show();
			
			if(visib_cb)
				visib_cb(true);
		}
	}

	void HideNetplayGUI() {
		if(netplay_window) {
			netplay_window->hide();
			
			if(visib_cb)
				visib_cb(false);
		}
	}

	bool Init(std::shared_ptr<Spelunky> sp, std::shared_ptr<DerandomizePatch> d, std::shared_ptr<Seeder> s, std::shared_ptr<GameHooks> gh) {
		spel = sp;
		dp = d;
		seeder = s;
		hooks = gh;

		sm = std::make_shared<SaveManager>(CurrentRCP(), hooks, dp);
		if(!sm->valid()) {
			DBG_EXPR(std::cout << "[GUINetplay] Invalid save manager." << std::endl);
			return false;
		}

		irp = std::make_shared<InputReceivePatch>(dp, gh);
		if(!irp->valid()) {
			DBG_EXPR(std::cout << "[GUINetplay] Invalid input receive patch." << std::endl);
			return false;
		}

		crp = std::make_shared<ConstantRandomPatch>(dp, gh, seeder);
		if(!crp->valid()) {
			DBG_EXPR(std::cout << "[GUINetplay] Invalid constant random patch." << std::endl);
			return false;
		}

		anticrash = std::make_shared<TempleAnticrash>(spel);
		if(!anticrash->valid()) {
			DBG_EXPR(std::cout << "[GUINetplay] Invalid temple anticrash patch." << std::endl);
			return false;
		}

		dcmp = std::make_shared<DisableCtrlMenuPatch>(spel);
		if(!dcmp->valid()) {
			DBG_EXPR(std::cout << "[GUINetplay] Invalid disable control menu patch." << std::endl);
			return false;
		}

		if(!AntipausePatch(spel).valid()) {
			DBG_EXPR(std::cout << "[GUINetplay] Invalid Antipause patch." << std::endl);
			return false;
		}

		init_window();
		return true;
	}

	int PlayerID() 
	{
		if(host_handle) {
			return 0;
		}
		else if(connect_handle) {
			return 1;
		}
		else if(session) {
			return session->get_pid();
		}
		else {
			return -1;
		}
	}
}