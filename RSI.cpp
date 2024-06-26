//
//  MACD.cpp
//  copass
//
//  Created by Akshad Mhaske on 09/02/24.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#define db double

using namespace std;

bool is_date_earlier(string d1,string d2){
    return d1<d2;
    
}

bool is_date_earlier_or_today(string d1,string d2){
    return d1<=d2;
    
}

string convert_to_d(string date){
    string res = "dd/mm/yyyy";
    res[0] = date[8];
    res[1] = date[9];
    res[3] = date[5];
    res[4] = date[6];
    res[6] = date[0];
    res[7] = date[1];
    res[8] = date[2];
    res[9] = date[3];
    return res;
}

string convert_to_y(string date){
    string res = "yyyy-mm-dd";
    res[0] = date[6];
    res[1] = date[7];
    res[2] = date[8];
    res[3] = date[9];
    res[5] = date[3];
    res[6] = date[4];
    res[8] = date[0];
    res[9] = date[1];
    return res;
}



void get_data(vector<pair<string,db>>&data){
    ifstream datafile("data_basic.csv");
    string line ;
    
    if(!datafile.is_open()){
        cerr << "could not open the file : data.csv" << endl;
    }
    vector<vector<string>>raw_data;
    int title_flag = 0;
    
    while(getline(datafile, line)) {
        if(title_flag==1){
            istringstream ss(line);
            string column;
            vector<string> row_data;

            while (std::getline(ss, column, ',')) {
                row_data.push_back(column);
            }

            raw_data.push_back(row_data);
        }
        else{
            title_flag = 1;
        }
        
    }
    datafile.close();
    
    
    
    int row_count = int(raw_data.size());
    for(int i = 0;i<row_count;i++){
        string date = convert_to_y(raw_data[i][0]);
        db close_price = stod(raw_data[i][1]);
        data.push_back({date,close_price});
    }
    reverse(data.begin(),data.end());
    
}

void solve(int x,int n,db oversold_threshold,db overbought_threshold,string start_date,string end_date,vector<pair<string,string>>&daily_cashflow,vector<vector<string>>&order_stats,vector<pair<string,db>>&data){
    
    int first_day = 0;
    while(is_date_earlier(data[first_day].first, start_date)){
        first_day++;
    }
    
    int last_day = first_day;
    while(last_day<data.size() && is_date_earlier_or_today(data[last_day].first, end_date)){
        last_day++;
    }
    last_day--;
    
    map<int,db>gain_loss;
    for(int curr = first_day - n +1;curr<=last_day;curr++){
        db diff = data[curr].second - data[curr-1].second;
        gain_loss[curr] = diff;
    }
    db gain_window = 0;
    db loss_window = 0;
    for(int curr = first_day-n+1;curr<=first_day;curr++){
        if(gain_loss[curr]>0){
            gain_window+=gain_loss[curr];
        }
        else{
            loss_window+=abs(gain_loss[curr]);
        }
    }
    db cash_in_hand = 0;
    db hold_quantity = 0;
    db avg_gain = gain_window/n;
    db avg_loss = loss_window/n;
    db RS;
    db RSI;
    if(avg_loss==0){
        RSI = 100;
    }
    else{
        RS = avg_gain/avg_loss;
        RSI = 100 - (100/(1+RS));
    }
    if(RSI<=oversold_threshold && hold_quantity<x){
        cash_in_hand-=data[first_day].second;
        daily_cashflow.push_back({data[first_day].first,to_string(cash_in_hand)});
        order_stats.push_back({data[first_day].first,"BUY","1",to_string(data[first_day].second)});
        hold_quantity++;
    }
    else if(RSI>=overbought_threshold && hold_quantity>(-1*x)){
        cash_in_hand+=data[first_day].second;
        daily_cashflow.push_back({data[first_day].first,to_string(cash_in_hand)});
        order_stats.push_back({data[first_day].first,"SELL","1",to_string(data[first_day].second)});
        hold_quantity--;
    }
    else{
        daily_cashflow.push_back({data[first_day].first,to_string(cash_in_hand)});
    }
    
    for(int current_day = first_day+1;current_day<=last_day;current_day++){
        db window_add = gain_loss[current_day];
        db window_remove = gain_loss[current_day-n];
        
        if(window_add>0){
            gain_window+=window_add;
        }
        else if(window_add<0){
            loss_window += abs(window_add);
        }
        
        if(window_remove>0){
            gain_window-=window_remove;
        }
        else if(window_remove<0){
            loss_window-=abs(window_remove);
        }
        
        db avg_gain = gain_window/n;
        db avg_loss = loss_window/n;
        db RS;
        db RSI;
        if(avg_loss==0){
            RSI = 100;
        }
        else{
            RS = avg_gain/avg_loss;
            RSI = 100 - (100/(1+RS));
        }
        if(RSI<oversold_threshold && hold_quantity<x){
            cash_in_hand-=data[current_day].second;
            daily_cashflow.push_back({data[current_day].first,to_string(cash_in_hand)});
            order_stats.push_back({data[current_day].first,"BUY","1",to_string(data[current_day].second)});
            hold_quantity++;
        }
        else if(RSI>overbought_threshold && hold_quantity>(-1*x)){
            cash_in_hand+=data[current_day].second;
            daily_cashflow.push_back({data[current_day].first,to_string(cash_in_hand)});
            order_stats.push_back({data[current_day].first,"SELL","1",to_string(data[current_day].second)});
            hold_quantity--;
        }
        else{
            daily_cashflow.push_back({data[current_day].first,to_string(cash_in_hand)});
        }
        
    }
    
    if(hold_quantity>0){
        cash_in_hand += data[last_day].second*hold_quantity;
        hold_quantity = 0;
        
    }
    else if(hold_quantity<0){
        cash_in_hand -= ((data[last_day].second)*abs(hold_quantity));
        hold_quantity = 0;
    }
    ofstream res_file("final_pnl.txt");
    res_file<<to_string(cash_in_hand)<<endl;
    res_file.close();
    
    
    
}


void write_data(vector<pair<string,string>>&daily_cashflow,vector<vector<string>>&order_stats){
    
    
    for(auto &x:daily_cashflow){
    	x.first = convert_to_d(x.first);
    }
    for(auto &x:order_stats){
    	x[0] = convert_to_d(x[0]);
    }
    ofstream dc_file("daily_cashflow.csv");
    dc_file<<"Date,Cashflow"<<endl;
    for(auto x:daily_cashflow){
        dc_file<<x.first<<","<<x.second<<endl;
    }
    dc_file.close();
    
    ofstream os_file("order_statistics.csv");
    
    os_file<<"Date,Order_dir,Quantity,Price"<<endl;
    for(auto row:order_stats){
        string temp;
        for(int i = 0;i<4;i++){
            temp+=row[i];
            if(i!=3){
                temp.push_back(',');
            }
        }
        os_file<<temp<<endl;
    }
    os_file.close();
    
}



    







int main(int argc,const char * argv[]){
    string symbol = argv[1];
    int x = stoi(argv[2]);
    int n = stoi(argv[3]);
    db oversold_threshold = stod(argv[4]);
    db overbought_threshold = stod(argv[5]);
    string start_date = argv[6];
    string end_date = argv[7];
    start_date = convert_to_y(start_date);
    end_date = convert_to_y(end_date);
    vector<pair<string,string>>daily_cashflow;
    vector<vector<string>>order_stats;
    vector<pair<string,db>> data;
    get_data(data);
    solve(x,n,oversold_threshold,overbought_threshold,start_date,end_date,daily_cashflow,order_stats,data);
    write_data(daily_cashflow,order_stats);
    
}

