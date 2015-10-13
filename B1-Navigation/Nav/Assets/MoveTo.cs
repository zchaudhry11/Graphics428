using UnityEngine;
using System.Collections;

public class MoveTo : MonoBehaviour {
    private NavMeshAgent navAgent;
    //private Animator anim;
    private bool select;
    private bool set = false;
    private Vector3 MoveD;
	// Use this for initialization
	void Start () {
        //anim = GetComponent<Animator>();
        navAgent = GetComponent<NavMeshAgent>();
	}
	
	// Update is called once per frame
	void Update () {
        if (select && set)
        {
            navAgent.destination = MoveD;
            //anim.SetTrigger("Move");
        }
        if (navAgent.pathStatus == NavMeshPathStatus.PathComplete)
        {
            set = false;
        }
	}

    void Select(int x)
    {
        Debug.Log("Player Logged");
        select = true;
    }

    void Deselect(int x)
    {
        Debug.Log("Player Delogged");
        select = false;
    }

    void Destination (Vector3 dest)
    {
        Debug.Log("Destination Set");
        MoveD = dest;
        set = true;
    }
    
}
