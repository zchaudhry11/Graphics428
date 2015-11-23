using UnityEngine;
using System.Collections;

public class GameController : MonoBehaviour
{
    private GameObject selectObject;
    public Material highlightedMat;
	
	void Update ()
    {
        if (Input.GetMouseButtonDown(0))
        {
            RaycastHit hit;
            if (Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hit, 100))
            {
                //Debug.DrawLine(Input.mousePosition, hit.point, Color.red, 2);
                if (hit.transform.tag == "Player")
                {
                    Debug.Log("Player Selected");
                    selectObject = hit.transform.gameObject;
                    if (selectObject != null)
                    {
                        selectObject.tag = "Selected";
                        selectObject.transform.GetChild(0).transform.gameObject.SetActive(false);
                        selectObject.transform.GetChild(4).transform.gameObject.SetActive(true);
                        selectObject.SendMessage("Select", 1);
                    }
                }
				if(hit.transform.tag == "Cube")
				{
                    Debug.Log("Cube Selected");
					selectObject = hit.transform.gameObject;
					if(selectObject != null)
					{
                        selectObject.tag = "Selected";
                        selectObject.SendMessage("Select");
					}
				}
				if (hit.transform.tag == "Ground" && selectObject != null)
                {
                    Debug.Log("Player Deselected");
                    GameObject[] deselect = GameObject.FindGameObjectsWithTag("Selected");
                    for (int i = 0; i < deselect.Length; i++)
                    {
                        deselect[i].tag = "Player";
                        deselect[i].transform.GetChild(0).transform.gameObject.SetActive(true);
                        deselect[i].transform.GetChild(4).transform.gameObject.SetActive(false);
                        deselect[i].SendMessage("Deselect", 1);
                    }
                }
                if (hit.transform.tag == null)
                {
                    Debug.Log("no tag found");
                }
            }
        }
        if (Input.GetMouseButtonDown(1))
        {
            RaycastHit hit;
            if (Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hit, 100))
            {
                if (hit.transform.tag != null && selectObject != null && GameObject.FindGameObjectsWithTag("Selected").Length != 0)
                {
                    Debug.Log("Destination Selected");
                    GameObject[] setDest = GameObject.FindGameObjectsWithTag("Selected");
                    for (int i = 0; i < setDest.Length; i++)
                    {
                        setDest[i].SendMessage("Destination", hit.point);
                    }
                }
            }
        }
	}
}