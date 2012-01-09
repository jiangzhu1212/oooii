// $(header)
#include <oBasis/oCSV.h>
#include <oBasis/oRef.h>
#include <cstring>

static const char* sTestCSV =
{
	"contract_id,seller_company_name,customer_company_name,customer_duns_number,contract_affiliate,FERC_tariff_reference,contract_service_agreement_id,contract_execution_date,contract_commencement_date,contract_termination_date,actual_termination_date,extension_provision_description,class_name,term_name,increment_name,increment_peaking_name,product_type_name,product_name,quantity,units_for_contract,rate,rate_minimum,rate_maximum,rate_description,units_for_rate,point_of_receipt_control_area,point_of_receipt_specific_location,point_of_delivery_control_area,point_of_delivery_specific_location,begin_date,end_date,time_zone" \
	"C71,The Electric Company,The Power Company,456543333,N,FERC Electric Tariff Original Volume No. 10,2,2/15/2001,2/15/2001,,,Evergreen,N/A,N/A,N/A,N/A,MB,ENERGY,0,, , , ,Market Based,,,,,,,,ES" \
	"C72,The Electric Company,Utility A,38495837,n,FERC Electric Tariff Original Volume No. 10,15,7/25/2001,8/1/2001,,,Evergreen,N/A,N/A,N/A,N/A,MB,ENERGY,0,, , , ,Market Based,,,,,,,,ES" \
	"C73,The Electric Company,Utility B,493758794,N,FERC Electric Tariff Original Volume No. 10,7,6/8/2001,7/6/2001,,,Evergreen,N/A,N/A,N/A,N/A,MB,ENERGY,0,, , , ,Market Based,,,, , ,,,ep" \
	"C74,The Electric Company,Utility C,594739573,n,FERC Electric Tariff Original Volume No. 10,25,6/8/2001,7/6/2001,,,Evergreen,N/A,N/A,N/A,N/A,MB,ENERGY,0,, , , ,Market Based,,,, , ,,,ep" \
	"C75,The Electric Company,The Power Company,456543333,N,FERC Electric Tariff Third Revised Volume No. 7,94,2/13/2001,7/1/2001,12/31/2006,,None,F,LT,M,P,T,ENERGY,2000,KWh,.1475, , ,Max amount of capacity and energy to be transmitted.  Bill based on monthly max delivery to City.,$/KWh,PJM,Point A,PJM,Point B,,,ep" \
	"C75,The Electric Company,The Power Company,456543333,N,FERC Electric Tariff Third Revised Volume No. 7,94,2/13/2001,7/1/2001,12/31/2006,,None,F,LT,M,P,T,point-to-point agreement,2000,KW,0.01, , ,,$/kw-mo,PJM,Point A,PJM,Point B,,,ep" \
	"C75,The Electric Company,The Power Company,456543333,N,FERC Electric Tariff Third Revised Volume No. 7,94,2/13/2001,7/1/2001,12/31/2006,,None,F,LT,M,P,T,network,2000,KW,0.2, , ,,$/kw-mo,PJM,Point A,PJM,Point B,,,ep" \
	"C75,The Electric Company,The Power Company,456543333,N,FERC Electric Tariff Third Revised Volume No. 7,94,2/13/2001,7/1/2001,12/31/2006,,None,F,LT,M,P,T,BLACK START SERVICE,2000,KW,0.22, , ,,$/kw-mo,PJM,Point A,PJM,Point B,,,ep" \
	"C75,The Electric Company,The Power Company,456543333,N,FERC Electric Tariff Third Revised Volume No. 7,94,2/13/2001,7/1/2001,12/31/2006,,None,F,LT,M,P,T,CAPACITY,2000,KW,0.04, , ,,$/kw-mo,PJM,Point A,PJM,Point B,,,ep" \
	"C75,The Electric Company,The Power Company,456543333,N,FERC Electric Tariff Third Revised Volume No. 7,94,2/13/2001,7/1/2001,12/31/2006,,None,F,LT,M,P,T,regulation & frequency response,2000,KW,0.1, , ,,$/kw-mo,PJM,Point A,PJM,Point B,,,ep" \
	"C75,The Electric Company,The Power Company,456543333,N,FERC Electric Tariff Third Revised Volume No. 7,94,2/13/2001,7/1/2001,12/31/2006,,None,F,LT,M,P,T,real power transmission loss,2000,KW,7, , ,,$/kw-mo,PJM,Point A,PJM,Point B,,,ep" \
	"C76,The Electric Company,The Power Company,456534333,N,FERC Electric Tariff Original Volume No. 10,132,12/15/2001,1/1/2002,12/31/2004,12/31/2004,None,F,LT,M,FP,MB,CAPACITY,70,MW,3750, , ,70MW for each and every hour over the term of the agreement (7x24 schedule).,$/MW,,,,,,,ep" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,35, , ,,$/MWH,,,PJM,Bus 4321,20020101,20030101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,37, , ,,$/MWH,,,PJM,Bus 4321,20030101,20040101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,39, , ,,$/MWH,,,PJM,Bus 4321,20040101,20050101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,41, , ,,$/MWH,,,PJM,Bus 4321,20050101,20060101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,43, , ,,$/MWH,,,PJM,Bus 4321,20060101,20070101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,45, , ,,$/MWH,,,PJM,Bus 4321,20070101,20080101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,47, , ,,$/MWH,,,PJM,Bus 4321,20080101,20090101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,49, , ,,$/MWH,,,PJM,Bus 4321,20090101,20100101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,51, , ,,$/MWH,,,PJM,Bus 4321,20100101,20110101,EP" \
	"C78,The Electric Company,\"The Electric Marketing Co., LLC\",23456789,Y,FERC Electric Tariff Original Volume No. 2,Service Agreement 1,1/2/1992,1/2/1992,1/1/2012,,Renewable annually by mutual agreement after termination date.,UP,LT,Y,FP,CB,ENERGY,0,MWH,53, , ,,$/MWH,,,PJM,Bus 4321,20110101,20120101,EP"
};

#define oTESTB(test, msg, ...) do { if (!(test)) return oErrorSetLast(oERROR_GENERIC, msg, ## __VA_ARGS__); } while(false)

bool oBasisTest_oCSV()
{
	oRef<threadsafe oCSV> csv;
	{
		size_t size = strlen(sTestCSV)+1;
		char* pBuffer = new char[size];
		strcpy_s(pBuffer, size, sTestCSV);
		oTESTB(oCSVCreate("Test CSV", pBuffer, &csv), "Failed to create test CSV");
		delete [] pBuffer;
	}

	const char* c = csv->GetElement(9,16);

	oErrorSetLast(oERROR_NONE, "");
	return true;
}
