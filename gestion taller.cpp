#include <iostream>
#include <string>
#include <vector>
#include <limits> 
#include "sqlite3.h"

using namespace std;

// ==========================================
// FUNCIONES AYUDANTES: VALIDACIÓN DE ERRORES
// ==========================================

// Obliga al usuario a ingresar un número entero y limpia el buffer
int leerEntero(string mensaje) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Limpia el salto de línea residual
            return valor;
        } else {
            cout << "[!] Error: Debes ingresar un numero valido.\n";
            cin.clear(); // Limpia la bandera de error
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Borra la palabra equivocada
        }
    }
}

// Obliga al usuario a ingresar un número decimal (double)
double leerDouble(string mensaje) {
    double valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        } else {
            cout << "[!] Error: Debes ingresar un numero decimal valido.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// Ejecuta instrucciones SQL en la base de datos de forma segura
void ejecutarSQL(sqlite3* DB, string instruccionSQL) {
    char* mensajeError;
    int resultado = sqlite3_exec(DB, instruccionSQL.c_str(), NULL, 0, &mensajeError);
    
    if (resultado != SQLITE_OK) {
        cout << "[!] Error en la base de datos: " << mensajeError << endl;
        sqlite3_free(mensajeError);
    }
}

// ==========================================
// CLASES DEL SISTEMA
// ==========================================

class Material {
private:
    string Tipo;
    string Color;
    double PesoGramos; 
    double PrecioPorGramo; 

public:
    Material(string tipo, string color, double pesoInicial, double precioG) {
        Tipo = tipo; Color = color; PesoGramos = pesoInicial; PrecioPorGramo = precioG;
    }

    string getTipo() { return Tipo; }
    double getPrecioPorGramo() { return PrecioPorGramo; }
    void setPrecioPorGramo(double nuevoPrecio) { PrecioPorGramo = nuevoPrecio; }
    
    // Función para chequear sin restar todavía
    bool tieneSuficiente(double gramosUsados) {
        return PesoGramos >= gramosUsados;
    }

    void descontarMaterial(double gramosUsados) {
        PesoGramos -= gramosUsados;
    }

    void mostrarInventario() {
        cout << "- " << Tipo << " (" << Color << ") | Disp: " << PesoGramos 
             << "g | Costo: $" << PrecioPorGramo << "/g" << endl;
    }
};

struct RegistroCompra { string pieza; double montoPagado; };

class Cliente {
private:
    string Nombre; string Apellido; string Direccion; string Telefono; 
    double DineroGenerado; 
    vector<RegistroCompra> Historial; 

public:
    Cliente(string nombre, string apellido, string direccion, string telefono) {
        Nombre = nombre; Apellido = apellido; Direccion = direccion; Telefono = telefono; DineroGenerado = 0.0;
    }
    string getNombre() { return Nombre + " " + Apellido; }
    void registrarCompra(string descripcion, double monto) {
        Historial.push_back({descripcion, monto}); DineroGenerado += monto;
    }
    void mostrarCliente() { cout << Nombre << " " << Apellido << " | Tel: " << Telefono << endl; }
};

struct RegistroTrabajo { string pieza; double horas; };

class Impresora {
private:
    string Modelo; double CostoPorHora; double HorasTotalesTrabajadas; 
    vector<RegistroTrabajo> TrabajosRealizados; 
public:
    Impresora(string modelo, double costoPorHora) {
        Modelo = modelo; CostoPorHora = costoPorHora; HorasTotalesTrabajadas = 0.0; 
    }
    string getModelo() { return Modelo; }
    double getCostoPorHora() { return CostoPorHora; }
    void setCostoPorHora(double nuevoCosto) { CostoPorHora = nuevoCosto; }
    void registrarTrabajo(string descripcion, double horas) {
        HorasTotalesTrabajadas += horas; TrabajosRealizados.push_back({descripcion, horas});
    }
    void mostrarEquipo() { cout << "- " << Modelo << " | Costo operativo: $" << CostoPorHora << "/h" << endl; }
};

// SOLUCIÓN A LA TRAMPA DE MEMORIA: Venta ahora toma una "fotografía" en texto de los datos
class Venta {
private:
    string Fecha; 
    string DescripcionPieza;
    string NombreClienteFoto; // Fotografía del nombre
    string MaterialFoto;      // Fotografía del material
    string EquipoFoto;        // Fotografía del equipo
    double GramosConsumidos;
    double HorasUso;
    double Total;

public:
    // Pasamos los objetos por referencia (&) solo para actualizarlos aquí adentro
    Venta(string fecha, string descripcion, Cliente& cliente, Material& material, Impresora& equipo, double gramos, double horas) {
        Fecha = fecha;
        DescripcionPieza = descripcion;
        GramosConsumidos = gramos;
        HorasUso = horas;
        
        // Guardamos las fotografías de texto para no depender de punteros fantasma
        NombreClienteFoto = cliente.getNombre();
        MaterialFoto = material.getTipo();
        EquipoFoto = equipo.getModelo();
        
        Total = (GramosConsumidos * material.getPrecioPorGramo()) + (HorasUso * equipo.getCostoPorHora());
        
        // Efectuamos los descuentos y registros reales en la memoria
        material.descontarMaterial(GramosConsumidos);
        equipo.registrarTrabajo(DescripcionPieza, HorasUso);
        cliente.registrarCompra(DescripcionPieza, Total);
    }

    double getTotal() { return Total; }
    string getFecha() { return Fecha; }
    string getDescripcion() { return DescripcionPieza; }
    string getNombreCliente() { return NombreClienteFoto; }

    void generarNotaEntrega() {
        cout << "\n=====================================" << endl;
        cout << "   NOTA DE ENTREGA - Fecha: " << Fecha << endl;
        cout << "=====================================" << endl;
        cout << "Cliente: " << NombreClienteFoto << endl;
        cout << "Pieza: " << DescripcionPieza << endl;
        cout << "Equipo: " << EquipoFoto << " (" << HorasUso << "h)" << endl;
        cout << "Material: " << GramosConsumidos << "g de " << MaterialFoto << endl;
        cout << "-------------------------------------" << endl;
        cout << "TOTAL A PAGAR: $" << Total << endl;
        cout << "=====================================\n" << endl;
    }
};

// ==========================================
// FUNCIONES DE CARGA DESDE BASE DE DATOS
// (Puestas aquí abajo para que C++ ya sepa qué es un Cliente, Material, etc.)
// ==========================================

// Lee los clientes de la base de datos y los carga en el sistema al iniciar
void cargarClientesDesdeDB(sqlite3* DB, vector<Cliente>& clientes) {
    string sql = "SELECT Nombre, Apellido, Direccion, Telefono FROM Clientes;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string nombre = (const char*)sqlite3_column_text(stmt, 0);
            string apellido = (const char*)sqlite3_column_text(stmt, 1);
            string direccion = (const char*)sqlite3_column_text(stmt, 2);
            string telefono = (const char*)sqlite3_column_text(stmt, 3);
            
            clientes.push_back(Cliente(nombre, apellido, direccion, telefono));
        }
    }
    sqlite3_finalize(stmt); 
}

// Lee los materiales de la base de datos y los carga en el inventario al iniciar
void cargarMaterialesDesdeDB(sqlite3* DB, vector<Material>& inventario) {
    string sql = "SELECT Tipo, Color, PesoGramos, PrecioPorGramo FROM Materiales;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string tipo = (const char*)sqlite3_column_text(stmt, 0);
            string color = (const char*)sqlite3_column_text(stmt, 1);
            double peso = sqlite3_column_double(stmt, 2);
            double precio = sqlite3_column_double(stmt, 3);

            inventario.push_back(Material(tipo, color, peso, precio));
        }
    }
    sqlite3_finalize(stmt);
}

// Lee las impresoras de la base de datos y las carga al iniciar
void cargarImpresorasDesdeDB(sqlite3* DB, vector<Impresora>& impresoras) {
    string sql = "SELECT Modelo, CostoPorHora FROM Impresoras;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string modelo = (const char*)sqlite3_column_text(stmt, 0);
            double costo = sqlite3_column_double(stmt, 1);

            impresoras.push_back(Impresora(modelo, costo));
        }
    }
    sqlite3_finalize(stmt);
}

// ==========================================
// FUNCIÓN PRINCIPAL
// ==========================================

int main() {
    sqlite3* DB; 
    int conexion = sqlite3_open("taller_real.db", &DB);

    if (conexion) {
        cout << "[!] Error al conectar con SQLite: " << sqlite3_errmsg(DB) << endl;
        return 0; // Detiene el programa si no se puede conectar
    } else {
        cout << "\n>>> [EXITO] BASE DE DATOS CREADA Y CONECTADA CORRECTAMENTE <<<\n" << endl;
    }

    string sqlTablas = 
        "CREATE TABLE IF NOT EXISTS Clientes ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Nombre TEXT NOT NULL, Apellido TEXT, Direccion TEXT, Telefono TEXT);"
        
        "CREATE TABLE IF NOT EXISTS Impresoras ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Modelo TEXT NOT NULL, CostoPorHora REAL);"
        
        "CREATE TABLE IF NOT EXISTS Materiales ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Tipo TEXT NOT NULL, Color TEXT, PesoGramos REAL, PrecioPorGramo REAL);"
        
        "CREATE TABLE IF NOT EXISTS Ventas ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Fecha TEXT, Descripcion TEXT, Cliente TEXT, Equipo TEXT, Material TEXT, "
        "Gramos REAL, Horas REAL, Total REAL);";

    ejecutarSQL(DB, sqlTablas);

    vector<Material> inventario; 
    vector<Cliente> clientes; 
    vector<Impresora> impresoras; 
    vector<Venta> historialVentas; 

    cargarMaterialesDesdeDB(DB, inventario);
    cargarClientesDesdeDB(DB, clientes);
    cargarImpresorasDesdeDB(DB, impresoras);
    
    int opcionPrincipal;

    do {
        cout << "\n========= MENU PRINCIPAL (VERSION LIMPIA) =========" << endl;
        cout << "1. Modulo de Clientes" << endl;
        cout << "2. Modulo de Impresoras" << endl;
        cout << "3. Modulo de Materiales" << endl;
        cout << "4. Registrar NUEVA VENTA" << endl;
        cout << "5. Modulo de REPORTES Y FINANZAS" << endl;
        cout << "6. Salir del Sistema" << endl;
        
        opcionPrincipal = leerEntero("Seleccione una opcion: ");

        switch (opcionPrincipal) {
            
            case 1: { 
                int opCliente;
                do {
                    cout << "\n--- MODULO CLIENTES ---" << endl;
                    cout << "1. Registrar nuevo Cliente\n2. Volver al Menu Principal" << endl;
                    opCliente = leerEntero("Opcion: ");
                    if (opCliente == 1) {
                        string nombre, apellido, direccion, telefono;
                        cout << "Nombre: "; getline(cin, nombre); 
                        cout << "Apellido: "; getline(cin, apellido);
                        cout << "Telefono: "; getline(cin, telefono); 
                        cout << "Direccion: "; getline(cin, direccion); 
                        
                        clientes.push_back(Cliente(nombre, apellido, direccion, telefono));
                        
                        string sqlInsert = "INSERT INTO Clientes (Nombre, Apellido, Direccion, Telefono) "
                                           "VALUES ('" + nombre + "', '" + apellido + "', '" + direccion + "', '" + telefono + "');";
                        ejecutarSQL(DB, sqlInsert);

                        cout << "-> Cliente guardado en el sistema.\n";
                    }
                } while (opCliente != 2);
                break;
            }

            case 2: { 
                int opImp;
                do {
                    cout << "\n--- MODULO IMPRESORAS ---" << endl;
                    cout << "1. Registrar nueva Impresora\n2. Volver al Menu Principal" << endl;
                    opImp = leerEntero("Opcion: ");
                    if (opImp == 1) {
                        string modelo; 
                        cout << "Modelo: "; getline(cin, modelo);
                        double costo = leerDouble("Costo operativo/hora ($): ");
                        
                        impresoras.push_back(Impresora(modelo, costo));

                        string sqlInsert = "INSERT INTO Impresoras (Modelo, CostoPorHora) VALUES ('" 
                                           + modelo + "', " + to_string(costo) + ");";
                        ejecutarSQL(DB, sqlInsert);

                        cout << "-> Impresora registrada y guardada en la base de datos.\n";
                    }
                } while (opImp != 2);
                break;
            }

            case 3: { 
                int opMat;
                do {
                    cout << "\n--- MODULO MATERIALES ---" << endl;
                    cout << "1. Ingresar nueva Bobina\n2. Volver al Menu Principal" << endl;
                    opMat = leerEntero("Opcion: ");
                    if (opMat == 1) {
                        string tipo, color; 
                        cout << "Tipo: "; getline(cin, tipo); 
                        cout << "Color: "; getline(cin, color); 
                        double peso = leerDouble("Peso inicial (g): "); 
                        double precio = leerDouble("Precio por gramo ($): "); 
                        
                        inventario.push_back(Material(tipo, color, peso, precio));

                        string sqlInsert = "INSERT INTO Materiales (Tipo, Color, PesoGramos, PrecioPorGramo) VALUES ('" 
                                           + tipo + "', '" + color + "', " + to_string(peso) + ", " + to_string(precio) + ");";
                        ejecutarSQL(DB, sqlInsert);

                        cout << "-> Material registrado y guardado en la base de datos.\n";
                    }
                } while (opMat != 2);
                break;
            }

            case 4: { 
                if (clientes.empty() || inventario.empty() || impresoras.empty()) {
                    cout << "\n[!] Faltan datos (cliente, material o impresora).\n"; break;
                }
                
                string fecha, desc;
                cout << "\n[NUEVA VENTA]\nFecha (ej. 09-2026): "; getline(cin, fecha);

                cout << "-- Seleccione Cliente --\n";
                for (size_t i = 0; i < clientes.size(); i++) cout << "[" << i << "] " << clientes[i].getNombre() << "\n";
                int iCli = leerEntero("Opcion: ");
                if (iCli < 0 || iCli >= (int)clientes.size()) { cout << "[!] Cliente no valido.\n"; break; }

                cout << "-- Seleccione Material --\n";
                for (size_t i = 0; i < inventario.size(); i++) cout << "[" << i << "] " << inventario[i].getTipo() << "\n";
                int iMat = leerEntero("Opcion: ");
                if (iMat < 0 || iMat >= (int)inventario.size()) { cout << "[!] Material no valido.\n"; break; }

                cout << "-- Seleccione Impresora --\n";
                for (size_t i = 0; i < impresoras.size(); i++) cout << "[" << i << "] " << impresoras[i].getModelo() << "\n";
                int iImp = leerEntero("Opcion: ");
                if (iImp < 0 || iImp >= (int)impresoras.size()) { cout << "[!] Impresora no valida.\n"; break; }

                cout << "Descripcion de la pieza: "; getline(cin, desc);
                double gramos = leerDouble("Gramos a consumir: ");
                
                if (!inventario[iMat].tieneSuficiente(gramos)) {
                    cout << "\n[!] ALERTA: Operacion cancelada. Solo te quedan " 
                         << inventario[iMat].tieneSuficiente(gramos) << "g de " << inventario[iMat].getTipo() << ".\n";
                    break;
                }

                double horas = leerDouble("Horas de impresion: ");

                Venta nuevaVenta(fecha, desc, clientes[iCli], inventario[iMat], impresoras[iImp], gramos, horas);
                nuevaVenta.generarNotaEntrega();
                historialVentas.push_back(nuevaVenta);
                
                string sqlInsertVenta = "INSERT INTO Ventas (Fecha, Descripcion, Cliente, Equipo, Material, Gramos, Horas, Total) "
                                        "VALUES ('" + fecha + "', '" + desc + "', '" + clientes[iCli].getNombre() + "', '" 
                                        + impresoras[iImp].getModelo() + "', '" + inventario[iMat].getTipo() + "', " 
                                        + to_string(gramos) + ", " + to_string(horas) + ", " + to_string(nuevaVenta.getTotal()) + ");";
                ejecutarSQL(DB, sqlInsertVenta);

                string sqlUpdateMaterial = "UPDATE Materiales SET PesoGramos = PesoGramos - " + to_string(gramos) + 
                                           " WHERE Tipo = '" + inventario[iMat].getTipo() + "';";
                ejecutarSQL(DB, sqlUpdateMaterial);
                
                break;
            }

            case 5: { 
                int opReporte;
                do {
                    cout << "\n--- MODULO DE REPORTES ---" << endl;
                    cout << "1. Reporte Historico y Financiero\n2. Volver" << endl;
                    opReporte = leerEntero("Opcion: ");

                    if (opReporte == 1) {
                        string filtroFecha;
                        cout << "Mes a consultar (ej. 09-2026) o TODOS: ";
                        getline(cin, filtroFecha);

                        double granTotal = 0.0; int clientesActivos = 0;

                        cout << "\n=== REPORTE (" << filtroFecha << ") ===" << endl;
                        for (size_t c = 0; c < clientes.size(); c++) {
                            double subtotal = 0.0; int trabajos = 0;
                            for (size_t v = 0; v < historialVentas.size(); v++) {
                                if (historialVentas[v].getNombreCliente() == clientes[c].getNombre()) {
                                    if (filtroFecha == "TODOS" || historialVentas[v].getFecha() == filtroFecha) {
                                        subtotal += historialVentas[v].getTotal(); trabajos++;
                                    }
                                }
                            }
                            if (trabajos > 0) {
                                clientesActivos++; granTotal += subtotal;
                                cout << "- " << clientes[c].getNombre() << " | Trabajos: " << trabajos << " | Total: $" << subtotal << endl;
                            }
                        }
                        cout << "-----------------------------------" << endl;
                        cout << "Clientes atendidos: " << clientesActivos << " | GRAN TOTAL: $" << granTotal << endl;
                    }
                } while (opReporte != 2);
                break;
            }
            case 6: cout << "\nCerrando..." << endl; break;
            default: cout << "\n[!] Opcion no valida." << endl;
        }
    } while (opcionPrincipal != 6);

    sqlite3_close(DB);
    return 0;
}