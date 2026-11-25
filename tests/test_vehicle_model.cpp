#include "VehicleModel.hpp"
#include <QCoreApplication>
#include <cmath>
#include <iostream>
#define CHECK(e) do { if (!(e)) { std::cerr<<"Error: " #e <<" line "<<__LINE__<<'\n'; return 1; } } while(false)
int main(int argc,char**argv) {
    QCoreApplication app(argc,argv);
    VehicleModel model; model.setExtent(450,250);
    MonitorSimulacion::Instantanea s;
    s.ejecucion=1; s.ejecutando=true; s.config.tiempoCruceMs=650;
    for(int i=1;i<=12;++i) {
        auto& v=s.vehiculos[i]; v.eje=Eje::NorteSur; v.etapa=1; v.llegada=0; v.tipo=i%3==0?"Bus":"Auto";
    }
    model.sync(s);
    auto value=[&](int row,int role){return model.data(model.index(row),role);};
    for(int f=0;f<70;++f) model.frame(0.016,f*16);
    CHECK(model.rowCount()==12);CHECK(model.hiddenNS()>0);CHECK(model.hiddenEO()==0);
    for(int row=0;row<12;++row) {
        CHECK(value(row,VehicleModel::Bus).toBool()==((row+1)%3==0));
        CHECK(value(row,VehicleModel::Stage).toInt()==1);
        const double half=value(row,VehicleModel::Bus).toBool()?30:22;
        CHECK(value(row,VehicleModel::Position).toDouble()+half<=-94+0.01);
        if(row>0) {
            const double precedingHalf=value(row-1,VehicleModel::Bus).toBool()?30:22;
            CHECK(value(row-1,VehicleModel::Position).toDouble()-precedingHalf-value(row,VehicleModel::Position).toDouble()-half>=7.99);
        }
    }
    auto before=value(0,VehicleModel::Position).toDouble();
    s.vehiculos[1].etapa=2;s.vehiculos[1].cruce=1120;model.sync(s);model.frame(0.016,1120);
    CHECK(std::abs(before-value(0,VehicleModel::Position).toDouble())<0.1);
    model.frame(0.016,1450);CHECK(value(0,VehicleModel::Position).toDouble()>before);
    CHECK(value(0,VehicleModel::Stage).toInt()==2);
    // El reloj visual no inventa un evento de paso aunque la instantánea se retrase.
    model.frame(0.016,1900);CHECK(value(0,VehicleModel::Stage).toInt()==2);
    s.vehiculos[1].etapa=3;s.vehiculos[1].salida=1770;model.sync(s);model.frame(0.016,1800);
    CHECK(value(0,VehicleModel::Position).toDouble()>=116);
    model.setReducedMotion(true);model.frame(0.016,1800);CHECK(!value(0,VehicleModel::Visible).toBool());
    // Un cambio de ejecución elimina todas las filas y las colas anteriores.
    s.ejecucion=2;s.vehiculos.clear();s.ejecutando=false;model.sync(s);model.frame(0.016,0);
    CHECK(model.rowCount()==0);CHECK(model.hiddenNS()==0);
    std::cout<<"Vehículos: tipos reales, separación, continuidad, estados y reinicio OK\n";
}
