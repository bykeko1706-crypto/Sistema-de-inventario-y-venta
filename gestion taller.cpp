#include <iostream>
#include <string>
#include <vector>
#include <limits> 
#include "sqlite3.h"

using namespace std;

// ==========================================
// 1. FUNCIONES AYUDANTES: VALIDACIÓN DE ERRORES
// ==========================================
// Explicación para tu defensa: "Estas funciones evitan que el programa colapse 
// si el usuario introduce una letra cuando el sistema espera un número".

// Obliga al usuario a ingresar un número entero y limpia el teclado
int leerEntero(string mensaje) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Limpia residuos como 'Enter'
            return valor;
        } else {
            cout << "[!] Error: Debes ingresar un numero valido.\n";
            cin.clear(); // Apaga la alerta de error de C++
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

// ==========================================
// 2. CONEXIÓN A LA BASE DE DATOS
// ==========================================
// Explicación: "Esta función es el puente que traduce las órdenes de C++ a SQLite".
void ejecutarSQL(sqlite3* DB, string instruccionSQL) {
    char* mensajeError;
    // sqlite3_exec ejecuta el comando. Si falla, guarda el motivo en mensajeError
    int resultado = sqlite3_exec(DB, instruccionSQL.c_str(), NULL, 0, &mensajeError);
    
    if (resultado != SQLITE_OK) {
        cout << "[!] Error en la base de datos: " << mensajeError << endl;
        sqlite3_free(mensajeError); // Libera la memoria del error para evitar fugas
    }
}

// ==========================================
// 3. ARQUITECTURA DEL SISTEMA (CLASES)
// ==========================================
// Explicación: "Aplicamos Programación Orientada a Objetos para modelar el entorno real".

class Material {
private:
    // Atributos privados por seguridad (Encapsulamiento)
    string Tipo;
    string Color;
    double PesoGramos; 
    double PrecioPorGramo; 

public:
    // Constructor: Nace el material con sus datos iniciales
    Material(string tipo, string color, double pesoInicial, double precioG) {
        Tipo = tipo; Color = color; PesoGramos = pesoInicial; PrecioPorGramo = precioG;
    }

    // Métodos Get/Set para acceder a la información de forma controlada
    string getTipo() { return Tipo; }
    double getPrecioPorGramo() { return PrecioPorGramo; }
    void setPrecioPorGramo(double nuevoPrecio) { PrecioPorGramo = nuevoPrecio; }
    
    // Verifica si hay suficientes gramos antes de imprimir la pieza
    bool tieneSuficiente(double gramosUsados) {
        return PesoGramos >= gramosUsados;
    }

    // Resta los gramos consumidos del inventario en memoria
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
    
    // Agrega la compra al historial interno del cliente
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
    
    // Suma horas de uso operativo de los equipos
    void registrarTrabajo(string descripcion, double horas) {
        HorasTotalesTrabajadas += horas; TrabajosRealizados.push_back({descripcion, horas});
    }
    void mostrarEquipo() { cout << "- " << Modelo << " | Costo operativo: $" << CostoPorHora << "/h" << endl; }
};

// Explicación: "La clase Venta captura los datos en texto para generar notas de entrega inmutables".
class Venta {
private:
    string Fecha; 
    string DescripcionPieza;
    string NombreClienteFoto; // Se guarda como texto (foto) para no depender de punteros
    string MaterialFoto;      
    string EquipoFoto;        
    double GramosConsumidos;
    double HorasUso;
    double Total;

public:
    // Los objetos entran por referencia (&) para poder modificarlos (ej: restar gramos)
    Venta(string fecha, string descripcion, Cliente& cliente, Material& material, Impresora& equipo, double gramos, double horas) {
        Fecha = fecha;
        DescripcionPieza = descripcion;
        GramosConsumidos = gramos;
        HorasUso = horas;
        
        // Guardamos las fotografías de texto
        NombreClienteFoto = cliente.getNombre();
        MaterialFoto = material.getTipo();
        EquipoFoto = equipo.getModelo();
        
        // El sistema calcula automáticamente el costo cruzando horas y gramos
        Total = (GramosConsumidos * material.getPrecioPorGramo()) + (HorasUso * equipo.getCostoPorHora());
        
        // Efectuamos los descuentos lógicos en la memoria temporal
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
// 4. FUNCIONES DE LECTURA (PERSISTENCIA DE DATOS)
// ==========================================
// Explicación: "Estos módulos recuperan la información del disco duro cuando arranca el programa".

void cargarClientesDesdeDB(sqlite3* DB, vector<Cliente>& clientes) {
    string sql = "SELECT Nombre, Apellido, Direccion, Telefono FROM Clientes;";
    sqlite3_stmt* stmt; // Puntero que guarda la respuesta de la base de datos
    
    // Si la consulta está bien escrita...
    if (sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        // ...leemos fila por fila mientras existan registros
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string nombre = (const char*)sqlite3_column_text(stmt, 0);
            string apellido = (const char*)sqlite3_column_text(stmt, 1);
            string direccion = (const char*)sqlite3_column_text(stmt, 2);
            string telefono = (const char*)sqlite3_column_text(stmt, 3);
            
            clientes.push_back(Cliente(nombre, apellido, direccion, telefono));
        }
    }
    sqlite3_finalize(stmt); // Cerramos el puntero para ahorrar memoria
}

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
// 5. FUNCIÓN PRINCIPAL (NÚCLEO DEL PROGRAMA)
// ==========================================

int main() {
    sqlite3* DB; 
    // Intenta abrir el archivo. Si no existe, lo crea automáticamente.
    int conexion = sqlite3_open("taller_real.db", &DB);

    if (conexion) {
        cout << "[!] Error al conectar con SQLite: " << sqlite3_errmsg(DB) << endl;
        return 0; // Aborta si no hay motor de datos
    } else {
        cout << "\n>>> [EXITO] BASE DE DATOS CREADA Y CONECTADA CORRECTAMENTE <<<\n" << endl;
    }

    // Estructuramos las tablas usando lenguaje SQL (Esquema relacional)
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

    // Vectores que almacenan la información activa en la RAM
    vector<Material> inventario; 
    vector<Cliente> clientes; 
    vector<Impresora> impresoras; 
    vector<Venta> historialVentas; 

    // Disparamos la lectura masiva de datos al iniciar el software
    cargarMaterialesDesdeDB(DB, inventario);
    cargarClientesDesdeDB(DB, clientes);
    cargarImpresorasDesdeDB(DB, impresoras);
    
    int opcionPrincipal;

    do {
        // Interfaz principal de usuario
        cout << "\n========= MENU PRINCIPAL (ESTUDIO PIXEL) =========" << endl;
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
                        
                        // Generamos el comando INSERT para hacer el registro permanente
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
                // Evitamos procesar ventas si el sistema está vacío
                if (clientes.empty() || inventario.empty() || impresoras.empty()) {
                    cout << "\n[!] Faltan datos (cliente, material o impresora).\n"; break;
                }
                
                string fecha, desc;
                cout << "\n[NUEVA VENTA]\nFecha (ej. 09-2026): "; getline(cin, fecha);

                // --- PROCESO DE SELECCIÓN POR ÍNDICE ---
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
                
                // Barrera de seguridad: Comprueba si la bobina soporta el trabajo
                if (!inventario[iMat].tieneSuficiente(gramos)) {
                    cout << "\n[!] ALERTA: Operacion cancelada. Solo te quedan " 
                         << inventario[iMat].tieneSuficiente(gramos) << "g de " << inventario[iMat].getTipo() << ".\n";
                    break;
                }

                double horas = leerDouble("Horas de impresion: ");

                // Se procesa la transacción lógica y se imprime recibo
                Venta nuevaVenta(fecha, desc, clientes[iCli], inventario[iMat], impresoras[iImp], gramos, horas);
                nuevaVenta.generarNotaEntrega();
                historialVentas.push_back(nuevaVenta);
                
                // --- ACTUALIZACIÓN DE TRANSACCIÓN EN BASE DE DATOS ---
                
                // 1. Guardamos el registro en la tabla de Ventas
                string sqlInsertVenta = "INSERT INTO Ventas (Fecha, Descripcion, Cliente, Equipo, Material, Gramos, Horas, Total) "
                                        "VALUES ('" + fecha + "', '" + desc + "', '" + clientes[iCli].getNombre() + "', '" 
                                        + impresoras[iImp].getModelo() + "', '" + inventario[iMat].getTipo() + "', " 
                                        + to_string(gramos) + ", " + to_string(horas) + ", " + to_string(nuevaVenta.getTotal()) + ");";
                ejecutarSQL(DB, sqlInsertVenta);

                // 2. Descontamos el material físico del inventario mediante UPDATE
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

                        // Recorremos los clientes y buscamos coincidencias en las ventas (Filtrado)
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
            case 6: cout << "\nCerrando sistema..." << endl; break;
            default: cout << "\n[!] Opcion no valida." << endl;
        }
    } while (opcionPrincipal != 6);

    sqlite3_close(DB); // Cerramos ordenadamente el acceso al archivo
    return 0;
}