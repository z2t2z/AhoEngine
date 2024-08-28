#include <IamAho.h>


class SandBox : public Aho::Application {
public:

	SandBox() {

	}

	~SandBox() {

	}

private:


};


Aho::Application* Aho::CreateApplication() {
	
	return new SandBox();

}