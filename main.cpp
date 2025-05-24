// ConsoleApplication7.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <cmath>
#include <iomanip>
#include <iostream>
#include<fstream>
#include <map>
#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <atlbase.h>
#include <json/json.h>
using namespace Json;
using namespace std;
double lambda;
double mu;
double sigma;
int stroki = 4;
Value load_config_file()
{
    string file_path = "simulation.config";
    ifstream file(file_path, ifstream::binary);
    if (!file.is_open()) {
        cerr << "Error: Could not open the file " << file_path << endl;
        return 1;
    }
    Value data;
    Json::StreamWriterBuilder wbuilder;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;
    if (!Json::parseFromStream(readerBuilder, file, &data, &errs)) {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
        return 1;
    }
    std::cout << data << std::endl;
    file.close();
    return data;
};
class message {
public:
    double time_arrive;
    double time_finish;
    int nomer;
    double time_take_for_start_work;
    int kolvo_proidennih_server;
    int tekush_server_stroka;
    int tekush_server_stolbec;
};
class core {
public:
    message current_message;
    int nomer;
    int busy;
    double time_finish_next_mes;
};

class server {
public:
    int nomer_end_core;
    double time_prostoy_full;
    double time_prostoy;
    double time_1_work;
    double time_2_work;
    double time_3_work;
    double time_4_work;
    vector<message> queue;
    vector<double> time_ochered;
    double time_last_take_mes;
    double time_last_insert_mes;
    double time_last_prostoy_end;
    double time_last_prostoy_end_dlya_zadachi;
    double sum_time_arrive_message;
    int kolvo_message;
    double time_last_arrive_message;
    vector<double> time_neprerivnaya_work;
    int nomer_neighbour_stroka[4];
    int nomer_neighbour_stolbec[4];
    int nomer_server[2];
    vector<core> cores;
};

class prohodka {
public:
    int nomer_core;
    int server_nomer_i;
    int server_nomer_j;
    double time_mes_finish;


};

void prohodka_sort(vector<prohodka>* sort_vector, double time_arrive)
{
    int size = (*sort_vector).size();
    if (size == 0)
        return;
    for (int i = 0; i < size - 1; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if ((*sort_vector)[i].time_mes_finish > time_arrive)
            {
                sort_vector->erase(sort_vector->begin() + i);
                size--;
                i--;
                break;
            }
            if ((*sort_vector)[i].time_mes_finish > (*sort_vector)[j].time_mes_finish)
            {
                prohodka temp;
                temp = (*sort_vector)[j];
                (*sort_vector)[j] = (*sort_vector)[i];
                (*sort_vector)[i] = temp;
            }
        }
    }
    return;
};
class system_all {
public:
    int n;
    int kolvo_cores;
    vector<vector<server>> servers;
    vector<double> queue_events;
    double full_time;
    int kappa;

    double message_arriving(message mes, int str, int stl, system_all* sys)
    {
        std::random_device rd{};
        std::mt19937 gen{ rd() };
        double ll = 1.0 / lambda;
        exponential_distribution<> dd(ll);
        vector<prohodka> prohodi;
        int flag_konec_prohoda = 1;

        while (flag_konec_prohoda == 1)
        {
            flag_konec_prohoda = 0;
            for (int i = 0; i < stroki; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    for (int k = 0; k < kolvo_cores; k++)
                    {
                        if (servers[i][j].cores[k].time_finish_next_mes < mes.time_arrive && servers[i][j].cores[k].time_finish_next_mes>0 && servers[i][j].cores[k].current_message.nomer != 0 && servers[i][j].cores[k].busy == 1)
                        {
                            flag_konec_prohoda = 1;
                            prohodka pohod;
                            pohod.nomer_core = k;
                            pohod.server_nomer_i = i;
                            pohod.server_nomer_j = j;
                            pohod.time_mes_finish = servers[i][j].cores[k].time_finish_next_mes;
                            prohodi.push_back(pohod);
                        }
                    }
                }
            }
            while (prohodi.size() > 0)
            {
                prohodka_sort(&prohodi, mes.time_arrive);
                for (int i = 0; i < stroki; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        double max_time_work = -1;
                        if (servers[i][j].queue.size() == 0)
                        {
                            for (int k = 0; k < kolvo_cores; k++)
                            {
                                if (servers[i][j].cores[k].time_finish_next_mes < mes.time_arrive)
                                {
                                    if (servers[i][j].cores[k].time_finish_next_mes > max_time_work)
                                        max_time_work = servers[i][j].cores[k].time_finish_next_mes;
                                }
                                else
                                {
                                    max_time_work = -1;
                                    break;
                                }
                            }
                            if (max_time_work >= 0)
                            {

                                servers[i][j].time_prostoy += (mes.time_arrive - max(max_time_work, servers[i][j].time_last_prostoy_end));
                                servers[i][j].time_last_prostoy_end = mes.time_arrive;
                            }
                        }
                    }
                }

                if (servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].current_message.nomer != 0)
                {
                    servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].current_message.kolvo_proidennih_server++;
                    if (servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].current_message.kolvo_proidennih_server < kappa)
                    {
                        int next = 0;
                        double max_prostoi = 0;

                        int min_queue = 100000000000;
                        for (int q = 0; q < 4; q++) {
                            if (servers[servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].nomer_neighbour_stroka[q]][servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                            {
                                min_queue = servers[servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].nomer_neighbour_stroka[q]][servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].nomer_neighbour_stolbec[q]].queue.size();
                                next = q;
                            }
                        }
                        double random_finish_next = dd(gen);
                        servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].current_message.time_arrive = servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].time_finish_next_mes;
                        servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].current_message.time_finish = random_finish_next;
                        message_arriving(servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].current_message, servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].nomer_neighbour_stroka[next], servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].nomer_neighbour_stolbec[next], sys);
                    }
                    if (servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].queue.size() > 0) {
                        while (servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].time_ochered.size() < servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].queue.size() + 1)
                        {
                            servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].time_ochered.push_back(0);
                        }
                        servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].time_ochered[servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].queue.size()] += (servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].time_finish_next_mes - max(servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].time_last_insert_mes, servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].time_last_take_mes));
                        servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].time_last_take_mes = servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].time_finish_next_mes;
                        servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].time_finish_next_mes += servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].queue[servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].queue.size() - 1].time_finish;
                        servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].current_message = servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].queue[servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].queue.size() - 1];
                        servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].queue.pop_back();
                    }
                    else
                    {
                        servers[prohodi[0].server_nomer_i][prohodi[0].server_nomer_j].cores[prohodi[0].nomer_core].busy = 0;
                    }
                }
                prohodi.clear();
                for (int i = 0; i < stroki; i++)
                {
                    for (int j = 0; j < n; j++)
                    {
                        for (int k = 0; k < kolvo_cores; k++)
                        {
                            if (servers[i][j].cores[k].time_finish_next_mes < mes.time_arrive && servers[i][j].cores[k].time_finish_next_mes>0 && servers[i][j].cores[k].current_message.nomer != 0 && servers[i][j].cores[k].busy == 1)
                            {
                                flag_konec_prohoda = 1;
                                prohodka pohod;
                                pohod.nomer_core = k;
                                pohod.server_nomer_i = i;
                                pohod.server_nomer_j = j;
                                pohod.time_mes_finish = servers[i][j].cores[k].time_finish_next_mes;
                                prohodi.push_back(pohod);
                            }
                        }
                    }
                }
            }

        }

        for (int i = 0; i < stroki; i++)
        {
            for (int j = 0; j < n; j++)
            {
                double max_time_work = -1;
                if (servers[i][j].queue.size() == 0)
                {
                    for (int k = 0; k < kolvo_cores; k++)
                    {
                        if (servers[i][j].cores[k].time_finish_next_mes < mes.time_arrive)
                        {
                            if (servers[i][j].cores[k].time_finish_next_mes > max_time_work)
                                max_time_work = servers[i][j].cores[k].time_finish_next_mes;
                        }
                        else
                        {
                            max_time_work = -1;
                            break;
                        }
                    }
                    if (max_time_work >= 0)
                    {

                        servers[i][j].time_prostoy += (mes.time_arrive - max(max_time_work, servers[i][j].time_last_prostoy_end));
                        servers[i][j].time_last_prostoy_end = mes.time_arrive;
                    }
                }
            }
        }

        vector<server> servers_end_before_mes_arrive;

        double min_end_time = 0;



        int i = str;
        int j = stl;
        servers[i][j].kolvo_message++;


        servers[i][j].sum_time_arrive_message += (mes.time_arrive - servers[i][j].time_last_arrive_message);
        servers[i][j].time_last_arrive_message = mes.time_arrive;
        vector<core> cores_early;

        for (int k = 0; k < kolvo_cores; k++)
        {
            if (servers[i][j].cores[k].time_finish_next_mes < mes.time_arrive)
            {
                cores_early.push_back(servers[i][j].cores[k]);
            }
        }
        if (cores_early.size() > 0)
        {
            if (cores_early.size() == 1)
            {
                if (servers[i][j].queue.size() == 0)
                {
                    if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                    {
                        servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                        if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                        {
                            int next = 0;
                            int min_queue = 100000000000;
                            for (int q = 0; q < 4; q++)
                            {
                                if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                {
                                    min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                    next = q;
                                }
                            }
                            double random_finish_next = dd(gen);
                            servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                            servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                            message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                        }
                    }
                    cores_early[0].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                    cores_early[0].busy = 1;
                    cores_early[0].current_message = mes;
                    servers[i][j].cores[cores_early[0].nomer] = cores_early[0];
                }
                else
                {
                    while (servers[i][j].queue.size() > 0 && cores_early[0].time_finish_next_mes < mes.time_arrive)
                    {

                        if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                        {
                            servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                            if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                            {
                                int next = 0;
                                int min_queue = 100000000000;
                                for (int q = 0; q < 4; q++)
                                {
                                    if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                    {
                                        min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                        next = q;
                                    }
                                }
                                double random_finish_next = dd(gen);
                                servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                            }
                        }
                        while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                        {
                            servers[i][j].time_ochered.push_back(0);
                        }
                        servers[i][j].time_ochered[servers[i][j].queue.size()] += (cores_early[0].time_finish_next_mes - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                        servers[i][j].time_last_take_mes = cores_early[0].time_finish_next_mes;
                        if (servers[i][j].queue.size() > 0) {
                            cores_early[0].time_finish_next_mes += servers[i][j].queue[servers[i][j].queue.size() - 1].time_finish;
                            cores_early[0].current_message = servers[i][j].queue[servers[i][j].queue.size() - 1];
                        }
                        else
                        {
                            cores_early[0].busy = 0;
                        }

                        if (servers[i][j].queue.size() > 0)
                            servers[i][j].queue.pop_back();
                    }
                    if (servers[i][j].queue.size() > 0)
                    {
                        if (cores_early[0].time_finish_next_mes == mes.time_arrive)
                        {
                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;
                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }
                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }

                            cores_early[0].current_message = mes;
                            cores_early[0].time_finish_next_mes += mes.time_finish;
                        }
                        else
                        {
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }

                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_insert_mes = mes.time_arrive;
                            servers[i][j].queue.push_back(mes);
                        }
                    }
                    else
                    {
                        if (cores_early[0].time_finish_next_mes <= mes.time_arrive)
                        {
                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;
                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }
                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }
                            cores_early[0].current_message = mes;
                            cores_early[0].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                            cores_early[0].busy = 1;
                        }
                        else {
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_insert_mes = mes.time_arrive;
                            servers[i][j].queue.push_back(mes);
                        }
                    }
                    servers[i][j].cores[cores_early[0].nomer] = cores_early[0];
                }
            }

            if (cores_early.size() == 2)
            {
                if (servers[i][j].queue.size() == 0)
                {
                    if (cores_early[0].time_finish_next_mes <= cores_early[1].time_finish_next_mes)
                    {
                        if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                        {
                            servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                            if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                            {
                                int next = 0;
                                int min_queue = 100000000000;
                                for (int q = 0; q < 4; q++)
                                {
                                    if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                    {
                                        min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                        next = q;
                                    }
                                }
                                double random_finish_next = dd(gen);
                                servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                            }
                        }
                        cores_early[0].current_message = mes;
                        cores_early[0].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                        cores_early[0].busy = 1;
                        servers[i][j].cores[cores_early[0].nomer] = cores_early[0];
                    }
                    else
                    {
                        if (cores_early[1].current_message.nomer != 0 && cores_early[1].busy == 1)
                        {
                            servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server++;
                            if (servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server < kappa)
                            {
                                int next = 0;
                                int min_queue = 100000000000;
                                for (int q = 0; q < 4; q++)
                                {
                                    if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                    {
                                        min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                        next = q;
                                    }
                                }
                                double random_finish_next = dd(gen);
                                servers[i][j].cores[cores_early[1].nomer].current_message.time_arrive = cores_early[1].time_finish_next_mes;
                                servers[i][j].cores[cores_early[1].nomer].current_message.time_finish = random_finish_next;
                                message_arriving(servers[i][j].cores[cores_early[1].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                            }
                        }
                        cores_early[1].current_message = mes;
                        cores_early[1].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                        cores_early[1].busy = 1;
                        servers[i][j].cores[cores_early[1].nomer] = cores_early[1];
                    }
                }
                else
                {
                    while (servers[i][j].queue.size() > 0 && (cores_early[0].time_finish_next_mes < mes.time_arrive || cores_early[1].time_finish_next_mes < mes.time_arrive))
                    {

                        if (cores_early[0].time_finish_next_mes <= cores_early[1].time_finish_next_mes)
                        {
                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;
                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }
                                    }
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            if (servers[i][j].queue.size() > 0) {
                                cores_early[0].current_message = servers[i][j].queue[servers[i][j].queue.size() - 1];
                                cores_early[0].time_finish_next_mes += servers[i][j].queue[servers[i][j].queue.size() - 1].time_finish;
                            }
                            else
                            {
                                cores_early[0].busy = 0;
                            }

                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (cores_early[0].time_finish_next_mes - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_take_mes = cores_early[0].time_finish_next_mes;

                        }
                        else
                        {
                            if (cores_early[1].current_message.nomer != 0 && cores_early[1].busy == 1)
                            {
                                servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;
                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }
                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[1].nomer].current_message.time_arrive = cores_early[1].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[1].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[1].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            if (servers[i][j].queue.size() > 0) {
                                cores_early[1].current_message = servers[i][j].queue[servers[i][j].queue.size() - 1];
                                cores_early[1].time_finish_next_mes += servers[i][j].queue[servers[i][j].queue.size() - 1].time_finish;
                            }
                            else
                            {
                                cores_early[1].busy = 0;
                            }
                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (cores_early[1].time_finish_next_mes - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_take_mes = cores_early[1].time_finish_next_mes;

                        }
                        if (servers[i][j].queue.size() > 0) {
                            servers[i][j].queue.pop_back();
                        }
                    }
                    if (servers[i][j].queue.size() > 0)
                    {
                        if (cores_early[0].time_finish_next_mes <= mes.time_arrive || cores_early[1].time_finish_next_mes <= mes.time_arrive)
                        {
                            if (cores_early[0].time_finish_next_mes <= cores_early[1].time_finish_next_mes)
                            {
                                if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                                {
                                    servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                    if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                    {
                                        int next = 0;
                                        int min_queue = 100000000000;
                                        for (int q = 0; q < 4; q++)
                                        {
                                            if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                            {
                                                min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                                next = q;
                                            }
                                        }
                                        double random_finish_next = dd(gen);
                                        servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                        servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                        message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                    }
                                }
                                cores_early[0].current_message = mes;
                                cores_early[0].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                                cores_early[0].busy = 1;
                                if (cores_early[1].time_finish_next_mes <= mes.time_arrive)
                                {

                                    if (cores_early[1].current_message.nomer != 0 && cores_early[1].busy == 1)
                                    {
                                        servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server++;
                                        if (servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server < kappa)
                                        {
                                            int next = 0;
                                            int min_queue = 100000000000;
                                            for (int q = 0; q < 4; q++)
                                            {
                                                if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                                {
                                                    min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                                    next = q;
                                                }
                                            }
                                            double random_finish_next = dd(gen);
                                            servers[i][j].cores[cores_early[1].nomer].current_message.time_arrive = cores_early[1].time_finish_next_mes;
                                            servers[i][j].cores[cores_early[1].nomer].current_message.time_finish = random_finish_next;
                                            cout << "from server[" << i << "][" << j << "]" << endl;
                                            message_arriving(servers[i][j].cores[cores_early[1].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                        }
                                    }
                                    while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                                    {
                                        servers[i][j].time_ochered.push_back(0);
                                    }
                                    if (servers[i][j].queue.size() > 0) {
                                        cores_early[1].current_message = servers[i][j].queue[servers[i][j].queue.size() - 1];
                                        cores_early[1].time_finish_next_mes += servers[i][j].queue[servers[i][j].queue.size() - 1].time_finish;
                                    }
                                    else
                                    {
                                        cores_early[1].busy = 0;
                                    }
                                    servers[i][j].time_ochered[servers[i][j].queue.size()] += (cores_early[1].time_finish_next_mes - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                                    servers[i][j].time_last_take_mes = cores_early[1].time_finish_next_mes;
                                    if (servers[i][j].queue.size() > 0) {
                                        servers[i][j].queue.pop_back();
                                    }
                                }
                            }
                            else
                            {
                                if (cores_early[1].current_message.nomer != 0 && cores_early[1].busy == 1)
                                {
                                    servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server++;
                                    if (servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server < kappa)
                                    {
                                        int next = 0;
                                        int min_queue = 100000000000;
                                        for (int q = 0; q < 4; q++)
                                        {
                                            if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                            {
                                                min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                                next = q;
                                            }
                                        }
                                        double random_finish_next = dd(gen);
                                        servers[i][j].cores[cores_early[1].nomer].current_message.time_arrive = cores_early[1].time_finish_next_mes;
                                        servers[i][j].cores[cores_early[1].nomer].current_message.time_finish = random_finish_next;
                                        message_arriving(servers[i][j].cores[cores_early[1].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                    }
                                }
                                cores_early[1].current_message = mes;
                                cores_early[1].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                                cores_early[1].busy = 1;
                                if (cores_early[0].time_finish_next_mes <= mes.time_arrive)
                                {

                                    if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                                    {
                                        servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                        if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                        {
                                            int next = 0;
                                            int min_queue = 100000000000;
                                            for (int q = 0; q < 4; q++)
                                            {
                                                if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                                {
                                                    min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                                    next = q;
                                                }
                                            }
                                            double random_finish_next = dd(gen);
                                            servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                            servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                            message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                        }
                                    }
                                    while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                                    {
                                        servers[i][j].time_ochered.push_back(0);
                                    }
                                    if (servers[i][j].queue.size() > 0) {
                                        cores_early[0].current_message = servers[i][j].queue[servers[i][j].queue.size() - 1];
                                        cores_early[0].time_finish_next_mes += servers[i][j].queue[servers[i][j].queue.size() - 1].time_finish;
                                    }
                                    else
                                    {
                                        cores_early[0].busy = 0;
                                    }
                                    servers[i][j].time_ochered[servers[i][j].queue.size()] += (cores_early[0].time_finish_next_mes - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                                    servers[i][j].time_last_take_mes = cores_early[0].time_finish_next_mes;

                                    if (servers[i][j].queue.size() > 0) {
                                        servers[i][j].queue.pop_back();
                                    }
                                }
                            }

                        }
                        else
                        {
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_insert_mes = mes.time_arrive;
                            servers[i][j].queue.push_back(mes);
                        }

                    }
                    else
                    {
                        if (cores_early[0].time_finish_next_mes <= cores_early[1].time_finish_next_mes)
                        {
                            if (cores_early[0].time_finish_next_mes <= mes.time_arrive)
                            {
                                if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                                {
                                    servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                    if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                    {
                                        int next = 0;
                                        int min_queue = 100000000000;
                                        for (int q = 0; q < 4; q++)
                                        {
                                            if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                            {
                                                min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                                next = q;
                                            }
                                        }
                                        double random_finish_next = dd(gen);
                                        servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                        servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                        message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                    }
                                }
                                cores_early[0].current_message = mes;
                                cores_early[0].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                                cores_early[0].busy = 1;
                            }
                            else
                            {
                                while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                                {
                                    servers[i][j].time_ochered.push_back(0);
                                }
                                servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                                servers[i][j].time_last_insert_mes = mes.time_arrive;
                                servers[i][j].queue.push_back(mes);
                            }
                        }
                        else
                        {
                            if (cores_early[1].time_finish_next_mes <= mes.time_arrive)
                            {
                                if (cores_early[1].current_message.nomer != 0 && cores_early[1].busy == 1)
                                {
                                    servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server++;
                                    if (servers[i][j].cores[cores_early[1].nomer].current_message.kolvo_proidennih_server < kappa)
                                    {
                                        int next = 0;
                                        int min_queue = 100000000000;
                                        for (int q = 0; q < 4; q++)
                                        {
                                            if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                            {
                                                min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                                next = q;
                                            }
                                        }
                                        double random_finish_next = dd(gen);
                                        servers[i][j].cores[cores_early[1].nomer].current_message.time_arrive = cores_early[1].time_finish_next_mes;
                                        servers[i][j].cores[cores_early[1].nomer].current_message.time_finish = random_finish_next;
                                        message_arriving(servers[i][j].cores[cores_early[1].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                    }
                                }
                                cores_early[1].current_message = mes;
                                cores_early[1].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                                cores_early[1].busy = 1;
                            }
                            else
                            {
                                while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                                {
                                    servers[i][j].time_ochered.push_back(0);
                                }
                                servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                                servers[i][j].time_last_insert_mes = mes.time_arrive;
                                servers[i][j].queue.push_back(mes);
                            }
                        }
                    }
                }
                servers[i][j].cores[cores_early[0].nomer] = cores_early[0];
                servers[i][j].cores[cores_early[1].nomer] = cores_early[1];
            }

            if (cores_early.size() > 2 && cores_early.size() < kolvo_cores)
            {
                for (int k = 0; k < cores_early.size() - 1; k++)
                {
                    for (int l = k + 1; l < cores_early.size(); l++)
                    {
                        if (cores_early[k].time_finish_next_mes > cores_early[l].time_finish_next_mes)
                        {
                            core temp = cores_early[k];
                            cores_early[k] = cores_early[l];
                            cores_early[l] = temp;
                        }
                    }
                }

                if (servers[i][j].queue.size() == 0)
                {
                    if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                    {
                        servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                        if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                        {
                            int next = 0;
                            int min_queue = 100000000000;
                            for (int q = 0; q < 4; q++)
                            {
                                if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                {
                                    min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                    next = q;
                                }
                            }
                            double random_finish_next = dd(gen);
                            servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                            servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                            message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                        }
                    }
                    cores_early[0].current_message = mes;
                    cores_early[0].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                    cores_early[0].busy = 1;
                    servers[i][j].cores[cores_early[0].nomer] = cores_early[0];
                }
                else
                {
                    while (servers[i][j].queue.size() > 0 && cores_early[0].time_finish_next_mes < mes.time_arrive)
                    {
                        while (cores_early[0].time_finish_next_mes < cores_early[1].time_finish_next_mes && servers[i][j].queue.size()>0)
                        {

                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;
                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }
                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            if (servers[i][j].queue.size() > 0) {
                                cores_early[0].current_message = servers[i][j].queue[servers[i][j].queue.size() - 1];
                                cores_early[0].time_finish_next_mes += servers[i][j].queue[servers[i][j].queue.size() - 1].time_finish;
                            }
                            else
                            {
                                cores_early[0].busy = 0;
                            }
                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (cores_early[0].time_finish_next_mes - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_take_mes = cores_early[0].time_finish_next_mes;

                            if (servers[i][j].queue.size() > 0) {
                                servers[i][j].queue.pop_back();
                            }
                        }
                        for (int k = 0; k < cores_early.size() - 1; k++)
                        {
                            for (int l = k + 1; l < cores_early.size(); l++)
                            {
                                if (cores_early[k].time_finish_next_mes > cores_early[l].time_finish_next_mes)
                                {
                                    core temp = cores_early[k];
                                    cores_early[k] = cores_early[l];
                                    cores_early[l] = temp;
                                }
                            }
                        }
                    }
                    if (servers[i][j].queue.size() == 0)
                    {
                        if (cores_early[0].time_finish_next_mes <= mes.time_arrive)
                        {
                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;
                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }

                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }
                            cores_early[0].current_message = mes;
                            cores_early[0].time_finish_next_mes = mes.time_arrive += mes.time_finish;
                            cores_early[0].busy = 1;
                        }
                        else
                        {
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_insert_mes = mes.time_arrive;
                            servers[i][j].queue.push_back(mes);
                        }
                    }
                    else
                    {
                        if (cores_early[0].time_finish_next_mes == mes.time_arrive)
                        {
                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;
                                    double max_prostoi = 0;

                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }

                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }
                            cores_early[0].current_message = mes;
                            cores_early[0].time_finish_next_mes = mes.time_arrive += mes.time_finish;
                            cores_early[0].busy = 1;
                        }
                        else
                        {
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_insert_mes = mes.time_arrive;
                            servers[i][j].queue.push_back(mes);
                        }
                    }
                }
                for (int k = 0; k < cores_early.size(); k++)
                {
                    servers[i][j].cores[cores_early[k].nomer] = cores_early[k];
                }
            }

            if (cores_early.size() == kolvo_cores)
            {
                for (int k = 0; k < cores_early.size() - 1; k++)
                {
                    for (int l = k + 1; l < cores_early.size(); l++)
                    {
                        if (cores_early[k].time_finish_next_mes > cores_early[l].time_finish_next_mes)
                        {
                            core temp = cores_early[k];
                            cores_early[k] = cores_early[l];
                            cores_early[l] = temp;
                        }
                    }
                }
                if (servers[i][j].queue.size() == 0)
                {
                    servers[i][j].time_neprerivnaya_work.push_back(cores_early[cores_early.size() - 1].time_finish_next_mes - servers[i][j].time_last_prostoy_end);
                    servers[i][j].time_last_prostoy_end = mes.time_arrive;
                    if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                    {
                        servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                        if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                        {
                            int next = 0;
                            double max_prostoi = 0;
                            int min_queue = 100000000000;
                            for (int q = 0; q < 4; q++)
                            {
                                if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                {
                                    min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                    next = q;
                                }

                            }
                            double random_finish_next = dd(gen);
                            servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                            servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                            message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                        }
                    }
                    cores_early[0].current_message = mes;
                    servers[i][j].time_prostoy += (mes.time_arrive - max(cores_early[cores_early.size() - 1].time_finish_next_mes, servers[i][j].time_last_prostoy_end));
                    servers[i][j].time_last_prostoy_end = mes.time_arrive;
                    cores_early[0].time_finish_next_mes = mes.time_arrive + mes.time_finish;
                    cores_early[0].busy = 1;
                    servers[i][j].cores[cores_early[0].nomer] = cores_early[0];
                }
                else
                {
                    while (servers[i][j].queue.size() > 0 && cores_early[0].time_finish_next_mes < mes.time_arrive)
                    {
                        while (cores_early[0].time_finish_next_mes < cores_early[1].time_finish_next_mes && servers[i][j].queue.size()>0)
                        {

                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;

                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }

                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }

                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }

                            if (servers[i][j].queue.size() > 0)
                            {
                                cores_early[0].current_message = servers[i][j].queue[servers[i][j].queue.size() - 1];
                                cores_early[0].time_finish_next_mes += servers[i][j].queue[servers[i][j].queue.size() - 1].time_finish;
                            }

                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (cores_early[0].time_finish_next_mes - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_take_mes = cores_early[0].time_finish_next_mes;

                            if (servers[i][j].queue.size() > 0)
                                servers[i][j].queue.pop_back();

                        }
                        for (int k = 0; k < cores_early.size() - 1; k++)
                        {
                            for (int l = k + 1; l < cores_early.size(); l++)
                            {
                                if (cores_early[k].time_finish_next_mes > cores_early[l].time_finish_next_mes)
                                {
                                    core temp = cores_early[k];
                                    cores_early[k] = cores_early[l];
                                    cores_early[l] = temp;
                                }
                            }
                        }
                    }
                    if (servers[i][j].queue.size() == 0)
                    {
                        if (cores_early[cores_early.size() - 1].time_finish_next_mes <= mes.time_arrive)
                        {
                            servers[i][j].time_neprerivnaya_work.push_back(cores_early[cores_early.size() - 1].time_finish_next_mes - servers[i][j].time_last_prostoy_end);
                            servers[i][j].time_prostoy += (mes.time_arrive - max(cores_early[cores_early.size() - 1].time_finish_next_mes, servers[i][j].time_last_prostoy_end));
                            servers[i][j].time_last_prostoy_end = mes.time_arrive;
                            servers[i][j].time_last_prostoy_end_dlya_zadachi = mes.time_arrive;
                        }
                        if (cores_early[0].time_finish_next_mes <= mes.time_arrive)
                        {
                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;
                                    double max_prostoi = 0;

                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() > min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }
                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }
                            cores_early[0].current_message = mes;
                            cores_early[0].time_finish_next_mes = mes.time_arrive += mes.time_finish;
                            cores_early[0].busy = 1;
                        }
                        else
                        {
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_insert_mes = mes.time_arrive;
                            servers[i][j].queue.push_back(mes);
                        }
                    }
                    else
                    {
                        if (cores_early[cores_early.size() - 1].time_finish_next_mes <= mes.time_arrive)
                        {
                            servers[i][j].time_neprerivnaya_work.push_back(cores_early[cores_early.size() - 1].time_finish_next_mes - servers[i][j].time_last_prostoy_end);
                            servers[i][j].time_last_prostoy_end = mes.time_arrive;
                            if (cores_early[0].current_message.nomer != 0 && cores_early[0].busy == 1)
                            {
                                servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server++;
                                if (servers[i][j].cores[cores_early[0].nomer].current_message.kolvo_proidennih_server < kappa)
                                {
                                    int next = 0;


                                    int min_queue = 100000000000;
                                    for (int q = 0; q < 4; q++)
                                    {
                                        if (servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size() < min_queue)
                                        {
                                            min_queue = servers[servers[i][j].nomer_neighbour_stroka[q]][servers[i][j].nomer_neighbour_stolbec[q]].queue.size();
                                            next = q;
                                        }

                                    }
                                    double random_finish_next = dd(gen);
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_arrive = cores_early[0].time_finish_next_mes;
                                    servers[i][j].cores[cores_early[0].nomer].current_message.time_finish = random_finish_next;
                                    message_arriving(servers[i][j].cores[cores_early[0].nomer].current_message, servers[i][j].nomer_neighbour_stroka[next], servers[i][j].nomer_neighbour_stolbec[next], sys);
                                }
                            }
                            cores_early[0].current_message = mes;
                            servers[i][j].time_prostoy += (mes.time_arrive - max(cores_early[cores_early.size() - 1].time_finish_next_mes, servers[i][j].time_last_prostoy_end));
                            servers[i][j].time_last_prostoy_end = mes.time_arrive;
                            cores_early[0].time_finish_next_mes = mes.time_arrive += mes.time_finish;
                            cores_early[0].busy = 1;
                        }
                        else
                        {
                            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
                            {
                                servers[i][j].time_ochered.push_back(0);
                            }
                            servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
                            servers[i][j].time_last_insert_mes = mes.time_arrive;
                            servers[i][j].queue.push_back(mes);
                        }
                    }

                    for (int k = 0; k < cores_early.size(); k++)
                    {
                        servers[i][j].cores[cores_early[k].nomer] = cores_early[k];
                    }
                }
            }
        }
        else
        {
            while (servers[i][j].time_ochered.size() < servers[i][j].queue.size() + 1)
            {
                servers[i][j].time_ochered.push_back(0);
            }
            servers[i][j].time_ochered[servers[i][j].queue.size()] += (mes.time_arrive - max(servers[i][j].time_last_insert_mes, servers[i][j].time_last_take_mes));
            servers[i][j].time_last_insert_mes = mes.time_arrive;
            servers[i][j].queue.push_back(mes);
        }
        (*sys).servers = servers;
        return mes.time_arrive;

    };
};


int main()
{
    int nomer_tek;
    int nomer_poyavilsya;
    double now_time = 0;
    double time;
    double lambda_start;
    double mu_start;
    double sigma_start;
    int score = 1000;
    int nn = 0;
    load_config_file();
    while (nn <= 0) {
        cout << "input n(n>0):";
        cin >> nn;
    }
    cout << "input start lambda, mu, lamba/mu<1" << endl;
    cout << "input lambda:";
    cin >> lambda_start;
    cout << "input standard deviation:" << endl;
    cin >> sigma_start;
    cout << "input mu:";
    cin >> mu_start;
    cout << "input Time:";
    cin >> time;
    cout << "input step for lambda:";
    double step_lambda;
    cin >> step_lambda;
    cout << "input step for sigma:";
    double step_sigma;
    cin >> step_sigma;
    cout << "input step for mu:";
    double step_mu;
    cin >> step_mu;
    cout << "input end for lambda:";
    double end_lambda;
    cin >> end_lambda;
    cout << "input end for sigma:";
    double end_sigma;
    cin >> end_sigma;
    cout << "input end for mu:";
    double end_mu;
    cin >> end_mu;
    int kolvo_cores = 4;
    std::random_device rd{};
    std::mt19937 gen{ rd() };



    vector <double> lambdas;
    vector <double> mus;
    vector <double> sigmas;
    vector <vector<double>> queues_razmer;
    vector <double> zanyatost;
    vector <double> prostoy;
    for (int c = 0; c < kolvo_cores; c++)
        zanyatost.push_back(0);
    double random_arrive;
    double random_finish;
    for (int l = 1; l <= nn; l++)
    {
        int score1 = 1000;
        system_all sys;
        sys.kolvo_cores = kolvo_cores;
        sys.full_time = score1 * time;

        sys.n = l;
        sys.kappa = l / 2 + 2;
        for (lambda = lambda_start; lambda <= end_lambda; lambda += step_lambda)
        {
            lambdas.push_back(lambda);

            for (mu = mu_start; mu <= end_mu; mu += step_mu)
            {
                mus.push_back(mu);

                for (sigma = sigma_start; sigma <= end_sigma; sigma += step_sigma)
                {
                    sigmas.push_back(sigma);
                    sys.servers.clear();
                    for (int i = 0; i < stroki; i++)
                    {
                        vector<server> row;
                        for (int j = 0; j < sys.n; j++)
                        {
                            server server;
                            server.time_prostoy_full = 0;
                            server.time_prostoy = 0;
                            server.time_1_work = 0;
                            server.time_2_work = 0;
                            server.time_3_work = 0;
                            server.time_4_work = 0;
                            server.nomer_server[0] = i;
                            server.nomer_server[1] = j;
                            server.time_last_take_mes = 0;
                            server.time_last_prostoy_end = 0;
                            server.time_last_prostoy_end_dlya_zadachi = 0;
                            server.time_last_insert_mes = 0;
                            server.time_ochered.resize(0, 0);
                            server.kolvo_message = 0;
                            server.sum_time_arrive_message = 0;
                            server.time_neprerivnaya_work.resize(0, 0);
                            server.time_last_arrive_message = 0;
                            if (i == 0)
                            {
                                server.nomer_neighbour_stroka[1] = 3;
                            }
                            else
                            {
                                server.nomer_neighbour_stroka[1] = i - 1;
                            }
                            server.nomer_neighbour_stolbec[1] = j;
                            if (j == 0)
                            {
                                server.nomer_neighbour_stolbec[0] = sys.n - 1;
                            }
                            else
                            {
                                server.nomer_neighbour_stolbec[0] = j - 1;
                            }
                            server.nomer_neighbour_stroka[0] = i;
                            if (i == 3)
                            {
                                server.nomer_neighbour_stroka[3] = 0;
                            }
                            else
                            {
                                server.nomer_neighbour_stroka[3] = i + 1;
                            }
                            server.nomer_neighbour_stolbec[3] = j;
                            if (j == sys.n - 1)
                            {
                                server.nomer_neighbour_stolbec[2] = 0;
                            }
                            else
                            {
                                server.nomer_neighbour_stolbec[2] = j + 1;
                            }
                            server.nomer_neighbour_stroka[2] = i;

                            row.push_back(server);
                        }
                        sys.servers.push_back(row);
                    }

                    score = 1000;
                    vector <vector<double>> time_arriving;
                    vector <vector<double>> time_finishing;
                    vector <vector<double>> time_start_vip;

                    while (score > 0) {
                        std::normal_distribution<double> d{ mu, sigma };
                        auto random = [&d, &gen] { return d(gen); };
                        double ll = 1.0 / lambda;
                        exponential_distribution<> dd(ll);


                        for (int i = 0; i < stroki; i++)
                            for (int j = 0; j < sys.n; j++)
                            {

                                sys.servers[i][j].nomer_end_core = -1;
                                sys.servers[i][j].queue.clear();
                                sys.servers[i][j].queue.resize(0);
                                sys.servers[i][j].time_last_prostoy_end = 0;
                                sys.servers[i][j].time_last_arrive_message = 0;
                                sys.servers[i][j].time_last_take_mes = 0;
                                sys.servers[i][j].time_prostoy = 0;
                                sys.servers[i][j].time_last_insert_mes = 0;
                                sys.servers[i][j].time_last_arrive_message = 0;
                                vector<core>cores;
                                for (int i = 0; i < kolvo_cores; i++)
                                {

                                    core core_1;
                                    core_1.busy = 0;
                                    core_1.time_finish_next_mes = 0;
                                    core_1.nomer = i;
                                    message new_mes;
                                    new_mes.nomer = 0;
                                    core_1.current_message = new_mes;
                                    cores.push_back(core_1);
                                }
                                sys.servers[i][j].cores = cores;
                            }
                        vector <double> time_arriving_step;
                        vector <double> time_finishing_step;
                        vector <double> time_start_vip_step;
                        vector <message> queue;
                        int start = 0;
                        int nomer = 1;
                        now_time = 0;

                        while (now_time <= time)
                        {
                            message mes;
                            do {
                                random_arrive = random();
                            } while (random_arrive <= 0);

                            if (time_arriving_step.size() > 0)
                            {
                                random_arrive += time_arriving_step[time_arriving_step.size() - 1];
                            }
                            else
                            {
                                start = 1;
                            }

                            time_arriving_step.push_back(random_arrive);

                            mes.time_arrive = random_arrive;
                            random_finish = dd(gen);

                            time_finishing_step.push_back(random_finish);

                            mes.time_finish = random_finish;
                            mes.tekush_server_stolbec = 0;
                            mes.tekush_server_stroka = 0;
                            mes.nomer = nomer;
                            nomer++;
                            mes.kolvo_proidennih_server = 0;
                            now_time = sys.message_arriving(mes, 0, 0, &sys);
                        }
                        double max_time_work = -1;
                        for (int i = 0; i < stroki; i++)
                            for (int j = 0; j < sys.n; j++)
                            {
                                while (sys.servers[i][j].time_ochered.size() < sys.servers[i][j].queue.size() + 1)
                                {
                                    sys.servers[i][j].time_ochered.push_back(0);
                                }
                                sys.servers[i][j].time_ochered[sys.servers[i][j].queue.size()] += now_time - max(sys.servers[i][j].time_last_insert_mes, sys.servers[i][j].time_last_take_mes);
                                if (sys.servers[i][j].queue.size() == 0)
                                {
                                    for (int k = 0; k < kolvo_cores; k++)
                                    {
                                        if (sys.servers[i][j].cores[k].time_finish_next_mes < now_time)
                                        {
                                            if (sys.servers[i][j].cores[k].time_finish_next_mes > max_time_work)
                                                max_time_work = sys.servers[i][j].cores[k].time_finish_next_mes;
                                        }
                                        else
                                        {
                                            max_time_work = -1;
                                            break;
                                        }
                                    }
                                    if (max_time_work >= 0)
                                    {

                                        sys.servers[i][j].time_prostoy += (now_time - max(max_time_work, sys.servers[i][j].time_last_prostoy_end));
                                        sys.servers[i][j].time_last_prostoy_end = now_time;
                                    }
                                }
                                sys.servers[i][j].time_prostoy_full += sys.servers[i][j].time_prostoy;
                            }
                        score--;
                        time_arriving.push_back(time_arriving_step);
                        time_finishing.push_back(time_finishing_step);

                    }
                    cout << sys.n << " " << lambda << " " << mu << " " << sigma << " " << endl; /*sys.time_prostoy / sys.full_time <<*/
                    for (int i = 0; i < stroki; i++)
                    {
                        for (int j = 0; j < sys.n; j++)
                        {
                            cout << "server[" << i << "][" << j << "]: ";
                            if (sys.servers[i][j].kolvo_message > 0) {
                                cout << sys.servers[i][j].time_prostoy_full / sys.full_time;
                            }
                            else
                            {
                                sys.servers[i][j].time_prostoy_full = sys.full_time;
                                cout << sys.servers[i][j].time_prostoy_full / sys.full_time;
                            }

                            prostoy.push_back(sys.servers[i][j].time_prostoy_full / sys.full_time);
                            double time_ochered_sum = 0;
                            for (int k = 0; k < sys.servers[i][j].time_ochered.size(); k++)
                            {
                                if (sys.servers[i][j].time_ochered[k] > 0)
                                    time_ochered_sum += sys.servers[i][j].time_ochered[k];
                            }
                            cout << " ";
                            if (sys.servers[i][j].kolvo_message >= 1)
                            {
                                double potok = 1 / (sys.servers[i][j].sum_time_arrive_message / sys.servers[i][j].kolvo_message);
                                cout << potok;
                            }
                            else
                            {
                                cout << "0";
                            }
                            cout << endl;
                            for (int k = 0; k < sys.servers[i][j].time_ochered.size(); k++)
                            {
                                //cout << "size=" << k << " time=" << sys.servers[i][j].time_ochered[k];
                                sys.servers[i][j].time_ochered[k] = sys.servers[i][j].time_ochered[k] / time_ochered_sum;
                                cout <</* << " veroaytnost="*/ " " << sys.servers[i][j].time_ochered[k] << " ";
                            }
                            cout << endl;
                            queues_razmer.push_back(sys.servers[i][j].time_ochered);
                            //system("pause");
                        }
                    }
                }
            }
        }
    }
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
